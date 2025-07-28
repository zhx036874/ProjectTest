QT += core
QT -= gui

CONFIG += c++14 console
CONFIG -= app_bundle

TARGET = AICodingTest
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/task.cpp \
    src/threadpool.cpp

HEADERS += \
    include/task.h \
    include/threadpool.h

INCLUDEPATH += \
    include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target