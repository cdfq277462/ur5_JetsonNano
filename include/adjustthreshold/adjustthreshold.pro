QT -= gui

TEMPLATE = lib
DEFINES += ADJUSTTHRESHOLD_LIBRARY

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    adjustthreshold.cpp

HEADERS += \
    adjustthreshold_global.h \
    adjustthreshold.h

# Default rules for deployment.
unix {
    target.path = /usr/lib

    INCLUDEPATH += /usr/local/include/opencv4
    LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
}

!unix {
    INCLUDEPATH += E:\opencv\build\include
}
!isEmpty(target.path): INSTALLS += target
