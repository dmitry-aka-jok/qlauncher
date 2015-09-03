#ifndef FOLDEREDITOR_H
#define FOLDEREDITOR_H

#include <QDialog>
#include <QLineEdit>

class FolderEditor : public QDialog
{
    Q_OBJECT

public:
    explicit FolderEditor(QWidget *parent = 0);
    ~FolderEditor();

public:
    void setName(const QString &name);
    QString getName();

private:
    QLineEdit dbname;
};

#endif // FOLDEREDITOR_H
