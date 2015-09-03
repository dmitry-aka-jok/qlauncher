#include "dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include <QDesktopWidget>
#include <QApplication>

#include "settings.h"
#include "databaseeditor.h"
#include "foldereditor.h"


Dialog::Dialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(&databasesTree);
    databasesTree.setColumnCount(2);
    databasesTree.setHeaderLabels(QStringList()<<QStringLiteral("Название")<<QStringLiteral("Путь"));
    databasesTree.setDragDropMode(QAbstractItemView::InternalMove);

    QAction* actionAddDatabase = new QAction(QStringLiteral("Добавить"), this);
    treeMenu.addAction(actionAddDatabase);
    connect(actionAddDatabase, SIGNAL(triggered()), this, SLOT(onActionAddDatabase()));

    QAction* actionAddCopy = new QAction(QStringLiteral("Добавить копию"), this);
    treeMenu.addAction(actionAddCopy);
    connect(actionAddCopy, SIGNAL(triggered()), this, SLOT(onActionAddCopy()));

    treeMenu.addSeparator();

    QAction* actionAddFolder = new QAction(QStringLiteral("Добавить папку"), this);
    treeMenu.addAction(actionAddFolder);
    connect(actionAddFolder, SIGNAL(triggered()), this, SLOT(onActionAddFolder()));

    treeMenu.addSeparator();
    QAction* actionEdit = new QAction(QStringLiteral("Изменить"),this);
    treeMenu.addAction(actionEdit);
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(onActionEdit()));

    QAction* actionDelete = new QAction(QStringLiteral("Удалить"),this);
    treeMenu.addAction(actionDelete);
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(onActionDelete()));

    treeMenu.addSeparator();

    actionShowName = new QAction(QStringLiteral("Показывать название"),this);
    actionShowName->setCheckable(true);
    treeMenu.addAction(actionShowName);
    connect(actionShowName, SIGNAL(triggered(bool)), this, SLOT(onActionShowName(bool)));

    actionShowPath = new QAction(QStringLiteral("Показывать путь"),this);
    actionShowPath->setCheckable(true);
    treeMenu.addAction(actionShowPath);
    connect(actionShowPath, SIGNAL(triggered(bool)), this, SLOT(onActionShowPath(bool)));


    databasesTree.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&databasesTree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onRequestContextMenu(const QPoint &)));

    options.setTitle(QStringLiteral("Настройки"));
    mainLayout->addWidget(&options);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    mainLayout->addLayout(buttonsLayout);

    btMore.setText(QStringLiteral("Настройки"));
    btMore.setCheckable(true);
    connect(&btMore, SIGNAL(toggled(bool)), this, SLOT(onButtonMoreToggled(bool)));
    buttonsLayout->addWidget(&btMore);

    buttonsLayout->addStretch();

    QPushButton* btAdd = new QPushButton(QStringLiteral("Добавить"));
    buttonsLayout->addWidget(btAdd);
    connect(btAdd, SIGNAL(clicked()), this, SLOT(onActionAddDatabase()));

    btLaunch.setText(QStringLiteral("Запустить"));
    buttonsLayout->addWidget(&btLaunch);
    btLaunch.setDefault(true);
    connect(&btLaunch, SIGNAL(clicked()), this, SLOT(accept()));


    connect(&databasesTree, SIGNAL(itemSelectionChanged()),
                this, SLOT(onSelectionChanged()));

    connect(&databasesTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
                this, SLOT(onDoubleClickOnTree(QTreeWidgetItem*,int)));

    QSettings *settings = Settings::getUserSettings();

    int sizew  = settings->value("mainwindow/size/w", 600).toInt();
    int sizeh   = settings->value("mainwindow/size/h", 400).toInt();
    QDesktopWidget *desktop = QApplication::desktop();
    int default_width   = int((desktop->width()-sizew)/2);
    int default_height  = int((desktop->height()-sizeh)/2);
    int posx    = settings->value("mainwindow/position/x", default_width).toInt();
    int posy    = settings->value("mainwindow/position/y", default_height).toInt();

    setGeometry(posx, posy, sizew, sizeh);

    btMore.setChecked(settings->value("mainwindow/more", false).toBool());
    onButtonMoreToggled(btMore.isChecked());

    options.setLayout(Settings::createSettingsLayout("global", "settings"));


    QTreeWidgetItem *item = databasesTree.invisibleRootItem();
    QString selectedPath = settings->value("mainwindow/tree/selectedPath").toString();
    int count = settings->beginReadArray("structure");
    QTreeWidgetItem *selectedItem = loadTree(item, settings, count, selectedPath);
    settings->endArray();


    bool isOn;
    isOn = settings->value("mainwindow/tree/columnNameVisible",true).toBool();
    actionShowName->setChecked(isOn);
    onActionShowName(isOn);
    isOn = settings->value("mainwindow/tree/columnPathVisible",true).toBool();
    actionShowPath->setChecked(isOn);
    onActionShowPath(isOn);


    int size = settings->beginReadArray("mainwindow/tree/columns");
    for (int i = 0; i < size; i++) {
        settings->setArrayIndex(i);
        databasesTree.setColumnWidth(i, settings->value("width",100).toInt());
    }
    settings->endArray();

    if(selectedItem)
        databasesTree.setCurrentItem(selectedItem);

    onSelectionChanged();

}

Dialog::~Dialog()
{
    QSettings *settings = Settings::getUserSettings();

    QRect geom = geometry();
    settings->setValue("mainwindow/position/x", geom.x());
    settings->setValue("mainwindow/position/y", geom.y());
    settings->setValue("mainwindow/size/w", geom.width());
    settings->setValue("mainwindow/size/h", geom.height());

    settings->beginWriteArray("mainwindow/tree/columns");
    for (int i = 0; i < databasesTree.columnCount(); i++) {
        settings->setArrayIndex(i);
        settings->setValue("width", databasesTree.columnWidth(i));
    }
    settings->endArray();

     settings->setValue("mainwindow/more", btMore.isChecked());

    Settings::saveSettingsLayout(&options, "settings");
    settings->sync();

    QTreeWidgetItem *item = databasesTree.invisibleRootItem();
    settings->beginWriteArray("structure");
    saveTree(item, settings);
    settings->endArray();
    settings->setValue("mainwindow/tree/columnNameVisible", !databasesTree.isColumnHidden(0));
    settings->setValue("mainwindow/tree/columnPathVisible", !databasesTree.isColumnHidden(1));

    settings->setValue("mainwindow/tree/selectedPath", databasesTree.currentItem()->text(1));
}

void Dialog::onButtonMoreToggled(bool state)
{
    options.setVisible(state);
}

void Dialog::onRequestContextMenu(const QPoint &p)
{
    treeMenu.exec(databasesTree.viewport()->mapToGlobal(p));
}

void Dialog::onActionAddDatabase()
{
        QTreeWidgetItem *parent;

        QTreeWidgetItem *selectedItem = databasesTree.currentItem();
           if(!selectedItem)
           {
               parent = databasesTree.invisibleRootItem();
           }
           else
           {
               if(selectedItem->text(1).isEmpty())
                   parent = selectedItem;
               else
                   parent = selectedItem->parent();

               if(!parent)
                   parent = databasesTree.invisibleRootItem();
           }

           DatabaseEditor editor; // = new DatabaseEditor();

           if(editor.exec()==QDialog::Accepted)
           {
               QTreeWidgetItem *newItem = new QTreeWidgetItem(parent);
               newItem->setText(0, editor.getName());
               newItem->setText(1, editor.getPath());
               newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
          }

           //delete editor;
}


void Dialog::onActionAddCopy()
{
        QTreeWidgetItem *parent;
        DatabaseEditor editor;// = new DatabaseEditor();

        QTreeWidgetItem *selectedItem = databasesTree.currentItem();
           if(!selectedItem)
           {
               parent = databasesTree.invisibleRootItem();
           }
           else
           {

               if(selectedItem->text(1).isEmpty())
                   parent = selectedItem;
               else
                   parent = selectedItem->parent();

               if(!parent)
                   parent = databasesTree.invisibleRootItem();

               editor.setName(selectedItem->text(0));
               editor.setPath(selectedItem->text(1));
           }


           if(editor.exec()==QDialog::Accepted)
           {
               QTreeWidgetItem *newItem = new QTreeWidgetItem(parent);
               newItem->setText(0, editor.getName());
               newItem->setText(1, editor.getPath());
               newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
          }

//           delete editor;
}


void Dialog::onActionAddFolder()
{
     QTreeWidgetItem *parent;

     QTreeWidgetItem *selectedItem = databasesTree.currentItem();
        if(!selectedItem)
        {
            parent = databasesTree.invisibleRootItem();
        }
        else
        {
            if(selectedItem->text(1).isEmpty())
                parent = selectedItem;
            else
                parent = selectedItem->parent();

            if(!parent)
                parent = databasesTree.invisibleRootItem();
        }

        FolderEditor editor;// = new FolderEditor();

        if(editor.exec()==QDialog::Accepted)
        {
            QTreeWidgetItem *newItem = new QTreeWidgetItem(parent);
            newItem->setText(0, editor.getName());
            newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
       }

//        delete editor;
}

void Dialog::onActionEdit()
{
    QTreeWidgetItem *selectedItem = databasesTree.currentItem();
    if(!selectedItem)
        return;

    if(selectedItem->text(1).isEmpty())
    {
        FolderEditor fe;// = new FolderEditor();
        fe.setName(selectedItem->text(0));
        if(fe.exec()==QDialog::Accepted)
        {
            selectedItem->setText(0, fe.getName());
        }
//        delete fe;
    }
    else
    {
        DatabaseEditor fe;// = new DatabaseEditor();
        fe.setName(selectedItem->text(0));
        fe.setPath(selectedItem->text(1));
        if(fe.exec()==QDialog::Accepted)
        {
            selectedItem->setText(0, fe.getName());
            selectedItem->setText(1, fe.getPath());
        }
 //       delete fe;
    }

}



QTreeWidgetItem* Dialog::loadTree(QTreeWidgetItem* item, QSettings* settings, int count, const QString &selectedPath)
{
    QTreeWidgetItem *itemForReturn=0;
    for(int i=0;i<count;++i)
    {
        settings->setArrayIndex(i);
        QTreeWidgetItem *child = new QTreeWidgetItem();
        child->setText(0, settings->value("name").toString());
        child->setText(1, settings->value("path").toString());
        item->addChild(child);

        if(child->text(1).isEmpty())
        {
            child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            int child_count = settings->beginReadArray("folder");
            QTreeWidgetItem *itemMaybeForReturn = loadTree(child, settings, child_count, selectedPath);
            if(itemMaybeForReturn)
                itemForReturn = itemMaybeForReturn;
            settings->endArray();
            child->setExpanded(settings->value("expanded").toBool());
        }
        else
        {
            child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        }

        if(child->text(1)==selectedPath)
            itemForReturn = child;
    }
    return itemForReturn;
}



void Dialog::saveTree(QTreeWidgetItem *item, QSettings *settings){

    for(int i=0;i<item->childCount();++i)
    {
        settings->setArrayIndex(i);
        QTreeWidgetItem *child = item->child(i);

        settings->setValue("name",child->text(0));
        settings->setValue("path",child->text(1));

        if(child->text(1).isEmpty())
        {
            settings->setValue("expanded",child->isExpanded());
            settings->beginWriteArray("folder");
            saveTree(child, settings);
            settings->endArray();
        }
    }
}

void Dialog::onSelectionChanged()
{
    QTreeWidgetItem *selectedItem = databasesTree.currentItem();
    if(selectedItem)
        btLaunch.setEnabled(!selectedItem->text(1).isEmpty());
    else
        btLaunch.setEnabled(false);

}


void Dialog::onDoubleClickOnTree(QTreeWidgetItem *item, int column)
{
    column = column;
    if(!item->text(1).isEmpty())
        emit accept();
}


void Dialog::onActionShowName(bool arg1)
{
    databasesTree.setColumnHidden(0, !arg1);
}

void Dialog::onActionShowPath(bool arg1)
{
    databasesTree.setColumnHidden(1, !arg1);
}

void Dialog::onActionDelete()
{
    QTreeWidgetItem *item = databasesTree.currentItem();
    QTreeWidgetItem *parent = item->parent();
    if(!parent)
        parent=databasesTree.invisibleRootItem();

    parent->removeChild(item);
    delete item;
}
