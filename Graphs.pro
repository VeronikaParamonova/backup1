QT       += core gui
QT       += sql
QT       += charts
QT       += core gui sql charts printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    barchart.cpp \
    iadapter.cpp \
    ichart.cpp \
    ioccontainer.cpp \
    jsonadaptee.cpp \
    jsonadapter.cpp \
    linechart.cpp \
    main.cpp \
    mainwindow.cpp \
    modeldata.cpp \
    sqladaptee.cpp \
    sqladapter.cpp

HEADERS += \
    barchart.h \
    iadapter.h \
    ichart.h \
    ioccontainer.h \
    jsonadaptee.h \
    jsonadapter.h \
    linechart.h \
    mainwindow.h \
    modeldata.h \
    sqladaptee.h \
    sqladapter.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
