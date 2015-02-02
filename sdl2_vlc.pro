TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    context.cpp

HEADERS += \
    context.h

include(deployment.pri)
qtcAddDeployment()

debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
    DEFINES += DEBUG
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
    DEFINES += RELEASE
}

msvc{
    error("Why are you using msvc?")
}

#sdl2 2.0.3
unix:!macx|win32: LIBS += -L$$PWD/../../sdl2_mingw/lib/x86/ -lSDL2

INCLUDEPATH += $$PWD/../../sdl2_mingw/include
DEPENDPATH += $$PWD/../../sdl2_mingw/include

##sdl2_image 2.0
unix:!macx|win32: LIBS += -L$$PWD/../../sdl2_image_vs/lib/x86/ -lSDL2_image
INCLUDEPATH += $$PWD/../../sdl2_image_vs/include
DEPENDPATH += $$PWD/../../sdl2_image_vs/include

#vlc
unix:!macx|win32: LIBS += -L$$PWD/../../libvlc/vlc-2.1.5/sdk/lib/ -llibvlc
INCLUDEPATH += $$PWD/../../libvlc/vlc-2.1.5/sdk/include
DEPENDPATH += $$PWD/../../libvlc/vlc-2.1.5/sdk/include
