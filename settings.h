#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QTextCodec>

#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include <QWidget>

#include <QDebug>

#define DBDRIVER QStringLiteral("QIBASE")

class Settings
{
public:
    static QString iniPath(){
        QString s = QCoreApplication::applicationDirPath()+"/"+QCoreApplication::applicationName();
        return s;
    }
    static QSettings* getUserSettings(){
        static QSettings s(iniPath()+"_user.ini", QSettings::IniFormat);
        s.setIniCodec(QTextCodec::codecForName("UTF-8"));
        return &s;
    }
    static QSettings* getSettings(){
        static QSettings s(iniPath()+".ini", QSettings::IniFormat);
        s.setIniCodec(QTextCodec::codecForName("UTF-8"));
        return &s;
    }

    static QFormLayout* createSettingsLayout(const QString& formUse, const QString& settingsStart)
    {
        QFormLayout *lay = new QFormLayout();

        QSettings *gs = Settings::getSettings();
        QSettings *us = Settings::getUserSettings();

        int cnt = gs->beginReadArray("ipkeys/key");
        for(int i=0;i<cnt;++i)
        {
            gs->setArrayIndex(i);
            QString id = gs->value("id").toString();
            if(gs->value("use").toString()==formUse)
            {
                if(gs->value("type").toString()=="logic")
                {
                    QCheckBox *box = new QCheckBox();
                    box->setText(gs->value("name").toString());
                    box->setWindowTitle(id);
                    box->setChecked(us->value(settingsStart+"/"+id,false).toBool());
                    lay->addRow(box);

                }
                if(gs->value("type","").toString()=="string")
                {
                    QLineEdit *edit = new QLineEdit();
                    edit->setWindowTitle(id);
                    edit->setText(us->value(settingsStart+"/"+id,"").toString());
                    lay->addRow(gs->value("name").toString(), edit);
                }
            }
        }
        gs->endArray();
        return lay;
    }



    static void activateSettingsLayout(QWidget* wg, bool active)
    {
        QList<QLineEdit *> lle = wg->findChildren<QLineEdit *>();
        QLineEdit *le;
        foreach (le, lle)
            le->setEnabled(active);

        QList<QCheckBox *> lbox = wg->findChildren<QCheckBox *>();
        QCheckBox *box;
        foreach (box, lbox)
            box->setEnabled(active);

    }


    static void saveSettingsLayout(QWidget* wg, const QString &settingsStart)
    {
        QSettings *us = Settings::getUserSettings();

        QList<QLineEdit *> lle = wg->findChildren<QLineEdit *>();
        QLineEdit *le;
        foreach (le, lle)
            us->setValue(settingsStart+"/"+le->windowTitle(),le->text());


        QList<QCheckBox *> lbox = wg->findChildren<QCheckBox *>();
        QCheckBox *box;
        foreach (box, lbox)
            us->setValue(settingsStart+"/"+box->windowTitle(),box->isChecked());

    }

private:
    Settings();
    Q_DISABLE_COPY(Settings)
};



#endif //SETTINGS_H
