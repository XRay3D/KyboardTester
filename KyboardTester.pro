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
DEFINES += EMU

#win32:RC_FILE = main_icon/myapp.rc

QMAKE_CXXFLAGS += /std:c++latest
QMAKE_CXXFLAGS += /await

SOURCES += \
    buttonmodel.cpp \
    connection.cpp \
    delegate.cpp \
    hwinterface/grbl.cpp \
    hwinterface/amk_tester.cpp \
    hwinterface/interface.cpp \
    main.cpp \
    mainwindow.cpp \
    matrixmodel.cpp \
    pointedit.cpp \
    tester.cpp




HEADERS += \
    buttonmodel.h \
    connection.h \
    delegate.h \
    hwinterface/grbl.h \
    hwinterface/amk_tester.h \
    hwinterface/common_interfaces.h \
    hwinterface/interface.h \
    mainwindow.h \
    matrixmodel.h \
    pointedit.h \
    tester.h


FORMS += \
        mainwindow.ui

include(../MyProtokol/myprotokol.pri)
