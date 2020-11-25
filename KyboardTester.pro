QT += core gui serialport widgets

TARGET = KyboardTester
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#win32:RC_FILE = main_icon/myapp.rc

QMAKE_CXXFLAGS += /std:c++latest
QMAKE_CXXFLAGS += /await

DESTDIR = $$_PRO_FILE_PWD_/bin

SOURCES += \
    buttonmodel.cpp \
    delegate.cpp \
    dialogconnection.cpp \
    hwinterface/amk_tester.cpp \
    hwinterface/grbl.cpp \
    hwinterface/interface.cpp \
    main.cpp \
    mainwindow.cpp \
    matrixmodel.cpp \
    pointedit.cpp \
    worker.cpp \


HEADERS += \
    buttonmodel.h \
    delegate.h \
    dialogconnection.h \
    hwinterface/amk_tester.h \
    hwinterface/common_interfaces.h \
    hwinterface/grbl.h \
    hwinterface/interface.h \
    mainwindow.h \
    matrixmodel.h \
    pointedit.h \
    worker.h \


FORMS += \
        dialogconnection.ui \
        mainwindow.ui

include(../MyProtokol/myprotokol.pri)
