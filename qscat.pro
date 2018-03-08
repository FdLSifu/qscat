#-------------------------------------------------
#
# Project created by QtCreator 2017-03-29T13:33:17
#
#-------------------------------------------------

QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qscat
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    scatool.cpp \
    curve.cpp \
    chartview.cpp \
    chart.cpp \
    synchrodialog.cpp \
    synchro.cpp \
    curvelistwidget.cpp \
    attackwindow.cpp \
    cpa.cpp \
    corrlistmodel.cpp

HEADERS  += mainwindow.h \
    scatool.h \
    curve.h \
    chartview.h \
    chart.h \
    synchrodialog.h \
    synchro.h \
    curvelistwidget.h \
    attackwindow.h \
    cpa.h \
    aes.h \
    corrlistmodel.h

FORMS    += design.ui \
    synchrodialog.ui \
    curvelistwidget.ui \
    attackwindow.ui

RESOURCES += \
    qscat_resource.qrc

DISTFILES +=
