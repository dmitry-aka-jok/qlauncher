#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T08:31:55
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qlauncher
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    foldereditor.cpp \
    databaseeditor.cpp

HEADERS  += dialog.h \
    settings.h \
    foldereditor.h \
    databaseeditor.h

FORMS    +=

RESOURCES += \
    resources.qrc

win32:RC_FILE = app.rc

OTHER_FILES += \
    app.rc
