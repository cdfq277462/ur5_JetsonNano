#-------------------------------------------------
#
# Project created by QtCreator 2021-04-15T11:34:52
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ur5_JetsonNano
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += OPENCV_DATA_DIR=\\\"/usr/local/share/opencv4/\\\"
DEFINES += TIME_MEASURE=1

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    camerathread.cpp \
        main.cpp \
        mainwindow.cpp \
    socket.cpp \

HEADERS += \
    camerathread.h \
    include/adjustthreshold.h \
    include/adjustthreshold_global.h \
        mainwindow.h \
    socket.h \

FORMS += \
        mainwindow.ui

INCLUDEPATH += /usr/include/opencv2 \
                /usr/include/librealsense2  \
                /usr/local/include/opencv4
LIBS += -L$$DESTDIR/ -lrealsense2
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_videoio -lopencv_objdetect -lopencv_dnn      \
    += -L./ -ladjustthreshold
