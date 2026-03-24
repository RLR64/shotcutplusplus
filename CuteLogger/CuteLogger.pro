QT       -= gui

TARGET = CuteLogger
TEMPLATE = lib

CONFIG += staticlib
CONFIG += create_prl

DEFINES += CUTELOGGER_LIBRARY

INCLUDEPATH += ./include

SOURCES += src/Logger.cpp \
           src/AbstractAppender.cpp \
           src/AbstractStringAppender.cpp \
           src/ConsoleAppender.cpp \
           src/FileAppender.cpp \
           src/RollingFileAppender.cpp

HEADERS += include/Logger.hpp \
           include/CuteLogger_global.hpp \
           include/AbstractAppender.hpp \
           include/AbstractStringAppender.hpp \
           include/ConsoleAppender.hpp \
           include/FileAppender.hpp \
           include/RollingFileAppender.hpp

win32 {
    SOURCES += src/OutputDebugAppender.cpp
    HEADERS += include/OutputDebugAppender.hpp
}

android {
    SOURCES += src/AndroidAppender.cpp
    HEADERS += include/AndroidAppender.hpp
}
