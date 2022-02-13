QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    LFCode/lfaudioplay.cpp \
    LFCode/lfcodec.cpp \
    LFCode/lfdecode.cpp \
    LFCode/lfdecodetask.cpp \
    LFCode/lfdemux.cpp \
    LFCode/lfdemux_task.cpp \
    LFCode/lfencode.cpp \
    LFCode/lfformat.cpp \
    LFCode/lfmux.cpp \
    LFCode/lfmuxtask.cpp \
    LFCode/lfsdl.cpp \
    LFCode/lftool.cpp \
    LFCode/lfvideoview.cpp \
    lfplayer.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    LFCode/lfaudioplay.h \
    LFCode/lfcodec.h \
    LFCode/lfdecode.h \
    LFCode/lfdecodetask.h \
    LFCode/lfdemux.h \
    LFCode/lfdemux_task.h \
    LFCode/lfencode.h \
    LFCode/lfformat.h \
    LFCode/lfmux.h \
    LFCode/lfmuxtask.h \
    LFCode/lfsdl.h \
    LFCode/lftool.h \
    LFCode/lfvideoview.h \
    lfplayer.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
win32 {
    FFMPEG_HOME = F:/Dev/msys64/usr/local/ffmpeg
    SDL_HOME = F:/Dev/SDL2-2.0.14/x86_64-w64-mingw32
}

macx {
    FFMPEG_HOME = /usr/local/ffmpeg
    SDL_HOME = /usr/local/Cellar/sdl2/2.0.16


INCLUDEPATH += $${FFMPEG_HOME}/include

LIBS += -L $${FFMPEG_HOME}/lib \
        -lavformat \
        -lavcodec \
        -lavutil \
        -lswresample \
        -lswscale

INCLUDEPATH += $${SDL_HOME}/include
LIBS += -L$${SDL_HOME}/lib \
        -lSDL2
        }
