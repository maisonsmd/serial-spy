QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

UI_DIR = src/views
INCLUDEPATH += src/

SOURCES += \
    src/main.cpp \
    src/controllers/mainwindow.cpp \
    src/controllers/serialhandler.cpp \
    src/models/historymodel.cpp \
    src/utils/loghandler.cpp

HEADERS += \
    src/controllers/mainwindow.h \
    src/controllers/serialhandler.h \
    src/models/historymodel.h \
    src/utils/loghandler.h

FORMS += \
    src/views/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
