#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTreeWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QMenu>
#include <QSettings>
#include <QAction>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


public slots:
    void onButtonMoreToggled(bool state);
    void onRequestContextMenu(const QPoint &p);
    void onActionAddDatabase();
    void onSelectionChanged();
    void onDoubleClickOnTree(QTreeWidgetItem *item, int column);
    void onActionShowName(bool arg1);
    void onActionShowPath(bool arg1);
    void onActionDelete();
    void onActionAddFolder();
    void onActionEdit();
    void onActionAddCopy();



private:
    QTreeWidget databasesTree;
    QGroupBox options;
    QPushButton btMore;
    QMenu treeMenu;
    QPushButton btLaunch;
    QAction* actionShowName;
    QAction* actionShowPath;


    QTreeWidgetItem* loadTree(QTreeWidgetItem* item, QSettings* settings, int count, const QString &selectedPath);
    void saveTree(QTreeWidgetItem *item, QSettings *settings);

};

#endif // DIALOG_H
