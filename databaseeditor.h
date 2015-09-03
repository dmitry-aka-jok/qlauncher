#ifndef DATABASEEDITOR_H
#define DATABASEEDITOR_H

#include <QDialog>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>

class DatabaseEditor : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseEditor(QWidget *parent = 0);
    ~DatabaseEditor();

public:
    void setName(const QString &name);
    QString getName();
    void setPath(const QString &path);
    QString getPath();

public slots:
    void onButtonMoreToggled(bool state);

    void onActionSelectFile();
    void loadDatabaseName();

private:
    QLineEdit dbname;
    QLineEdit dbpath;

    QGroupBox options;
    QPushButton btMore;

};

#endif // DATABASEEDITOR_H
