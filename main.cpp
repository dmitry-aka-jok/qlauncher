#include "dialog.h"
#include <QApplication>

#include <QMessageBox>
#include <QRegularExpression>
#include <QSplashScreen>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <QVariant>
#include <QDir>

#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("qlauncher");

    QString appDatabase;
    QStringList appParams;

    QStringList arguments = QCoreApplication::arguments();

    bool isNeedSettings = false;

    QSettings *ls  = Settings::getUserSettings();
    QSettings *gs  = Settings::getSettings();
    int cnt = gs->beginReadArray("ipkeys/key");
    gs->endArray();
    if(cnt==0)
    {
        QSettings *defaults = new QSettings(QStringLiteral(":/defaults/default.ini"), QSettings::IniFormat);
        defaults->setIniCodec(QTextCodec::codecForName("UTF-8"));
        QStringList defaultKeys = defaults->allKeys();

        for (int i = 0; i < defaultKeys.count(); i++)
            gs->setValue(defaultKeys.at(i), defaults->value(defaultKeys.at(i)));

        delete defaults;
    }

    QFont font("Arial", 16);
    QApplication::setFont(font);


    QList<QString> ignoreMap;
    ignoreMap<<"-F"<<"-DCREATEDB";


    if(arguments.count()<2)
    {
        Dialog* w = new Dialog();
        int res = w->exec();

        if(res==QDialog::Rejected)
            return 0;

        delete w;

        appDatabase = ls->value("mainwindow/tree/selectedPath").toString();

        isNeedSettings = true;
    }
    else
    {
        // check arguments for db
        for(int i=1;i<arguments.count();++i)
        {
            QString test = arguments.at(i);
            if(test.contains(":")&&!test.startsWith("-"))
            {

                if (test.indexOf(":")==1 && test.lastIndexOf(":")==1)
                    test = QStringLiteral("localhost:%1").arg(test);

                appDatabase = test;

                if(arguments.count()==2)
                    isNeedSettings = true;
            }
            else
            {
                bool isIgnoringParam = false;
                for(int j=0;j<ignoreMap.count() && !isIgnoringParam;++j)
                   if(test.startsWith(ignoreMap.at(j))) isIgnoringParam = true;

                if(!isIgnoringParam)
                    appParams.append(test);
            }
        }
        /*
        if(appPath.isEmpty())
        {
            appUuid = ls->value("mainwindow/tree/selectedUuid").toString();
            appPath = ls->value("database/uuid-"+appUuid+"/path").toString();
        }
        */

    }

//    QString appLocalFolder = appDatabase.replace(QRegularExpression("(:|/|\\)"),"_");4
    QString appLocalFolder = appDatabase;
    appLocalFolder.replace(QRegularExpression(":|/|\\\\"),"_");

    if(isNeedSettings)
    {
        int cnt = gs->beginReadArray("ipkeys/key");

        for(int i=0;i<cnt;++i)
        {
            gs->setArrayIndex(i);
            QString id = gs->value("id").toString();

            //qDebug()<<gs->value("name").toString()<<gs->value("type").toString()<<(gs->value("type").toString()==QStringLiteral("logic"));
            if(gs->value("type").toString()==QStringLiteral("logic"))
            {
                bool is_selected = false;
                if(gs->value("use").toString()==QStringLiteral("local"))
                    is_selected = ls->value(QStringLiteral("database/%1/%2").arg(appLocalFolder).arg(id),false).toBool();
                else
                    is_selected = ls->value(QStringLiteral("settings/%1").arg(id),false).toBool();

                //qDebug()<<id<<is_selected<<gs->value("type").toString()<<gs->value("name").toString();
                if(is_selected)
                {
                    QString param = gs->value("launch").toString();
                    if(param.contains("%1"))
                        param = param.arg(""); // no data for logic
                    appParams.append(param);
                }
            }

            if(gs->value("type").toString()==QStringLiteral("string"))
            {
                QString text;
                if(gs->value("use").toString()==QStringLiteral("local"))
                    text = ls->value(QStringLiteral("database/%1/%2").arg(appLocalFolder).arg(id)).toString();
                else
                    text = ls->value(QStringLiteral("settings/%1").arg(id)).toString();

 //               qDebug()<<id<<!text.isEmpty()<<text;
                if(!text.isEmpty())
                {
                    QString param = gs->value("launch").toString();
                    if(param.contains("%1"))
                        param = param.arg(text);
                    //qDebug()<<param;
                    appParams.append(param);
                }
            }

        }
        gs->endArray();

    }


    if(appDatabase.isEmpty()){
        QMessageBox::critical(0,
                              QStringLiteral("Ошибка"),
                              QStringLiteral("Не указан путь к базе данных")
                              );

        return 2;
    }

    QPixmap pixmap(":/pics/splash.gif");
    QSplashScreen splash;
    splash.setPixmap(pixmap);
    splash.show();
    //qDebug()<<

    splash.showMessage(QStringLiteral("\nЗагрузка..."), Qt::AlignHCenter);

    QSqlDatabase db = QSqlDatabase::addDatabase(DBDRIVER);
    db.setDatabaseName(appDatabase);

    QString test;

    QString username = "sysdba";
    test = ls->value(QStringLiteral("database/%1/user").arg(appLocalFolder)).toString();
    if(!test.isEmpty())username = test;
    test = ls->value(QStringLiteral("settings/user")).toString();
    if(!test.isEmpty())username = test;
    db.setUserName(username);

    QString password = "masterkey";
    test = ls->value(QStringLiteral("password/%1/user").arg(appLocalFolder)).toString();
    if(!test.isEmpty())password = test;
    test = ls->value(QStringLiteral("settings/password")).toString();
    if(!test.isEmpty())password = test;
    db.setPassword(password);

    if(!db.open()){
        QMessageBox::critical(0,
                              QStringLiteral("Ошибка"),
                              QStringLiteral("Не удалось подключиться к базе данных\nТест ошибки:%1\n").arg(db.lastError().text())
                              );
        return 3;
    }

    QString appToLaunch = "IP2.EXE";
    test = ls->value(QStringLiteral("database/%1/application").arg(appLocalFolder)).toString();
    if(!test.isEmpty())password = test;
    test = ls->value(QStringLiteral("settings/application")).toString();
    if(!test.isEmpty())password = test;

    QSqlQuery query;
    query.exec("select sum(datasize) from ip$filestorage where not path like '%\\%'");
    query.next();
    int fullSize = query.value(0).toInt();
    int loaded   = 0;
    query.clear();

    query.exec("select id, path, crc, datasize,flags from ip$filestorage where not path like '%\\%'");
    QDir dir(QCoreApplication::applicationDirPath());
    if(!dir.exists("dbconnectors"))
        dir.mkdir("dbconnectors");
    dir.cd("dbconnectors");

    if(!dir.exists(appLocalFolder)){
        dir.mkdir(appLocalFolder);
        dir.cd(appLocalFolder);
    }else{
        dir.cd(appLocalFolder);
    }
    QString appFolder = dir.absolutePath();
    bool isAppToLaunch = false;

    while (query.next())
    {
        QString name = query.value(1).toString();
        QFile file(dir.filePath(name));
        if(QString::compare(name, appToLaunch, Qt::CaseInsensitive))
            isAppToLaunch = true;


        bool isLoadThis = false;
        if(file.exists())
        {
            if (!file.open(QIODevice::ReadOnly))
            {
                isLoadThis = true;
            }
            else
            {
                if(file.size()==query.value(3).toInt())
                {
                    QFile file_crc(dir.filePath(name+".crc"));
                    if (!file_crc.open(QIODevice::ReadOnly))
                        isLoadThis = true;
                    else{
                        if(file_crc.readAll()!=query.value(2).toByteArray())
                            isLoadThis = true;
                        file_crc.close();
                    }
                }else
                    isLoadThis = true;
                file.close();
            }
        }
        else
        {
            isLoadThis = true;
        }

        if(isLoadThis)
        {
            QSqlQuery queryData;
            queryData.exec("select data from ip$filestorage where id = "+query.value(0).toString());
            queryData.next();

            if (!file.open(QIODevice::WriteOnly))
                continue;

            bool isCompressed = query.value(4).toInt()==1;
            if (isCompressed){
                QByteArray ba;
                ba.insert(0,query.value(3).toInt());
                for(int i=ba.length();i<4;++i)
                    ba.insert(0,char(0));
                ba.append(queryData.value(0).toByteArray());
                ba = qUncompress(ba);
                file.write(ba);
            }else
                file.write(queryData.value(0).toByteArray());

//            file.write(queryData.value(0).toByteArray());
            file.close();
            QFile file_crc(dir.filePath(name+".crc"));
            if (!file_crc.open(QIODevice::WriteOnly))
                continue;
            file_crc.write(query.value(2).toByteArray());
            file_crc.close();
        }

        loaded += query.value(3).toInt();

        int rr = (int) loaded * 100 / fullSize ;
        splash.showMessage(QStringLiteral("\nЗагрузка...(%1 %)").arg(rr), Qt::AlignHCenter);
    }

    splash.showMessage(QStringLiteral("\nЗапуск"), Qt::AlignHCenter);

    if(isAppToLaunch){
#ifdef Q_OS_WIN
        QSettings registry("HKEY_CURRENT_USER\\Software\\Novasoft\\IP\\Launcher",QSettings::NativeFormat);
        QString app = QApplication::applicationFilePath();
        registry.setValue("Path", app);
        registry.sync();
#endif

        appParams.append(appDatabase);

#ifdef Q_OS_LINUX
        appParams.insert(0, appToLaunch);
        appToLaunch = "wine";
#endif
        bool res = QProcess::startDetached(appToLaunch, appParams, appFolder);
        if(!res){
            QMessageBox::critical(0,
                                  QStringLiteral("Ошибка"),
                                  QStringLiteral("Не удалось запустить программу\n%1 %2").arg(appToLaunch).arg(appParams.join(" ")));
            return 4;
        }

    }


   // splash.finish(0);

    return 0;//a.exec();
}
