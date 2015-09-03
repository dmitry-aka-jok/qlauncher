#include "foldereditor.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDesktopWidget>
#include <QApplication>

#include "settings.h"

FolderEditor::FolderEditor(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    QFormLayout* formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    //QLineEdit* dbname = new QLineEdit();
    formLayout->addRow(QStringLiteral("Название"), &dbname);

    QPushButton* btCancel   = new QPushButton("Отмена");
    QPushButton* btOk         = new QPushButton("ОК");
    QHBoxLayout* btLayout  = new QHBoxLayout();

    btLayout->addWidget(btCancel);
    btLayout->addStretch();
    btLayout->addWidget(btOk);
    QWidget* wg = new QWidget();
    wg->setLayout(btLayout);

    mainLayout->addStretch();
    mainLayout->addWidget(wg);

    connect(btOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btCancel, SIGNAL(clicked()), this, SLOT(reject()));

    btOk->setDefault(true);

    QSettings *settings = Settings::getUserSettings();

    int sizew  = settings->value("foldereditor/size/w", 600).toInt();
    int sizeh   = settings->value("foldereditor/size/h", 400).toInt();
    QDesktopWidget *desktop = QApplication::desktop();
    int default_width   = int((desktop->width()-sizew)/2);
    int default_height  = int((desktop->height()-sizeh)/2);
    int posx    = settings->value("foldereditor/position/x", default_width).toInt();
    int posy    = settings->value("foldereditor/position/y", default_height).toInt();

    setGeometry(posx, posy, sizew, sizeh);
}

FolderEditor::~FolderEditor()
{
    QSettings *settings = Settings::getUserSettings();

    QRect geom = geometry();
    settings->setValue("foldereditor/position/x", geom.x());
    settings->setValue("foldereditor/position/y", geom.y());
    settings->setValue("foldereditor/size/w", geom.width());
    settings->setValue("foldereditor/size/h", geom.height());
}

void FolderEditor::setName(const QString &name)
{
    dbname.setText(name);
}

QString FolderEditor::getName()
{
    return dbname.text();
}
