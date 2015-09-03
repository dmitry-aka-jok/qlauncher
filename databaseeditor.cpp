#include "databaseeditor.h"

#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QRegularExpression>
#include <QPushButton>
#include <QFileDialog>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


#include "settings.h"

DatabaseEditor::DatabaseEditor(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    QFormLayout* formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    QWidget *wg = new QWidget();
    wg->setContentsMargins(0,0,0,0);
    QPushButton* btSelect = new QPushButton(QStringLiteral("Выбрать"));
    connect(btSelect, SIGNAL(clicked()), this, SLOT(onActionSelectFile()));
    QHBoxLayout* la = new QHBoxLayout();
    la->addWidget(&dbpath);
    connect(&dbpath, SIGNAL(editingFinished()), this, SLOT(loadDatabaseName()));

    la->addWidget(btSelect);
    la->setContentsMargins(0,0,0,0);
    wg->setLayout(la);

    formLayout->addRow(QStringLiteral("Путь"), wg);

    formLayout->addRow(QStringLiteral("Название"), &dbname);


    mainLayout->addStretch();

    options.setTitle(QStringLiteral("Настройки"));
    mainLayout->addWidget(&options);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonsLayout);

    btMore.setText(QStringLiteral("Настройки"));
    btMore.setCheckable(true);
    connect(&btMore, SIGNAL(toggled(bool)), this, SLOT(onButtonMoreToggled(bool)));
    buttonsLayout->addWidget(&btMore);


    QPushButton* btCancel   = new QPushButton("Отмена");
    QPushButton* btOk         = new QPushButton("ОК");

    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btCancel);
    buttonsLayout->addWidget(btOk);

    connect(btOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btCancel, SIGNAL(clicked()), this, SLOT(reject()));

    btOk->setDefault(true);

    QSettings *settings = Settings::getUserSettings();

    int sizew  = settings->value("databaseeditor/size/w", 600).toInt();
    int sizeh   = settings->value("databaseeditor/size/h", 400).toInt();
    QDesktopWidget *desktop = QApplication::desktop();
    int default_width   = int((desktop->width()-sizew)/2);
    int default_height  = int((desktop->height()-sizeh)/2);
    int posx    = settings->value("databaseeditor/position/x", default_width).toInt();
    int posy    = settings->value("databaseeditor/position/y", default_height).toInt();

    setGeometry(posx, posy, sizew, sizeh);

    btMore.setChecked(settings->value("databaseeditor/more", false).toBool());
    onButtonMoreToggled(btMore.isChecked());

    //TODO возможно что то стоит переделать с дефолтами
    options.setLayout(Settings::createSettingsLayout("local", QStringLiteral("database/default")));

}

DatabaseEditor::~DatabaseEditor()
{
    QSettings *settings = Settings::getUserSettings();

    QRect geom = geometry();
    settings->setValue("databaseeditor/position/x", geom.x());
    settings->setValue("databaseeditor/position/y", geom.y());
    settings->setValue("databaseeditor/size/w", geom.width());
    settings->setValue("databaseeditor/size/h", geom.height());

    settings->setValue("databaseeditor/more", btMore.isChecked());

    QString appLocalFolder = getPath();
    appLocalFolder.replace(QRegularExpression(":|/|\\\\"),"_");

    Settings::saveSettingsLayout(&options, QStringLiteral("database/%1").arg(appLocalFolder));
    settings->sync();
}

void DatabaseEditor::setName(const QString &name)
{
    dbname.setText(name);
}

QString DatabaseEditor::getName()
{
    return dbname.text();
}

void DatabaseEditor::setPath(const QString &path)
{
    dbpath.setText(path);

    QString appLocalFolder = path;
    appLocalFolder.replace(QRegularExpression(":|/|\\\\"),"_");

    delete options.layout();
    options.setLayout(Settings::createSettingsLayout("local", QStringLiteral("database/%1").arg(appLocalFolder)));
}

QString DatabaseEditor::getPath()
{
    return dbpath.text();
}

void DatabaseEditor::onButtonMoreToggled(bool state)
{
    options.setVisible(state);
}

void DatabaseEditor::onActionSelectFile()
{
    QSettings *settings = Settings::getUserSettings();
    QString path = settings->value("databaseeditor/lastFolder").toString();

    QString str = QFileDialog::getOpenFileName(this, QStringLiteral("Укажите базу данных"), path, QStringLiteral("Базы данных(*.fdb *.gdb);;Все файлы(*)"));
    if(str.contains(":/"))
        str.replace("/","\\");

    if(!str.isEmpty())
    {
        dbpath.setText(QStringLiteral ("localhost:%1").arg(str));
        loadDatabaseName();
        settings->setValue("databaseeditor/lastFolder", str);
    }
}

void DatabaseEditor::loadDatabaseName()
{
    if(getPath().isEmpty() || !getName().isEmpty())
        return;

    {
       QSqlDatabase db = QSqlDatabase::addDatabase(DBDRIVER, "test");

        db.setDatabaseName(getPath());
        QSettings *ls = Settings::getUserSettings();

        QString test;

        QString username = "sysdba";
        test = ls->value("settings/user").toString();
        if(!test.isEmpty())
            username = test;
        QLineEdit *leUser = options.findChild<QLineEdit *>("user");
        if(leUser)
            test = leUser->text();
        if(!test.isEmpty())
            username = test;

        db.setUserName(username);

        QString password = "masterkey";
        test = ls->value("settings/password").toString();
        if(!test.isEmpty())
            password = test;
        QLineEdit *lePass = options.findChild<QLineEdit *>("password");
        if(lePass)
            test = lePass->text();
        if(!test.isEmpty())
            password = test;

        db.setPassword(password);

        if(db.open())
        {
            QSqlQuery *query = new QSqlQuery("select first 1 title from ip$databaseparams", db);
            query->next();
            setName(query->value(0).toString());
            delete query;
        }
    }
    QSqlDatabase::removeDatabase("test");
}

