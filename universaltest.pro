#-------------------------------------------------
#
# Project created by QtCreator 2016-06-07T19:35:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = universaltest
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    subwidget.cpp \
    childwindow.cpp \
    ../Test/errordialog.cpp \
    aisubwidget.cpp \
    diosubwidget.cpp \
    ctrsubwidget.cpp \
    tmrdialog.cpp \
    testutilities.cpp \
    aosubwidget.cpp \
    discoversubwidget.cpp \
    qcustomplot.cpp \
    datamanager.cpp \
    trigdialog.cpp \
    dataselectdialog.cpp \
    queuedialog.cpp \
    eventsdialog.cpp

HEADERS  += mainwindow.h \
    subwidget.h \
    childwindow.h \
    ul.h \
    ../Test/errordialog.h \
    unitest.h \
    aisubwidget.h \
    diosubwidget.h \
    ctrsubwidget.h \
    tmrdialog.h \
    testutilities.h \
    aosubwidget.h \
    discoversubwidget.h \
    qcustomplot.h \
    datamanager.h \
    trigdialog.h \
    dataselectdialog.h \
    queuedialog.h \
    eventsdialog.h

FORMS    += mainwindow.ui \
    subwidget.ui \
    ../Test/errordialog.ui \
    aisubwidget.ui \
    diosubwidget.ui \
    ctrsubwidget.ui \
    tmrdialog.ui \
    aosubwidget.ui \
    discoversubwidget.ui \
    trigdialog.ui \
    dataselectdialog.ui \
    queuedialog.ui \
    eventsdialog.ui

DISTFILES +=

unix:!macx: LIBS += -L$$PWD/./ -luldaq

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.