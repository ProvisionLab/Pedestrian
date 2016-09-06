QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = detectorVehiccles
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

#QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
#QMAKE_LFLAGS_RELEASE -= -O1

ICON += images/icon.ico

#LIBS += -LC:\caffe\Build\Win32\Release -lcaffe_dll

CONFIG += warn_off
QMAKE_CFLAGS_WARN_ON -= -Wall
QMAKE_CXXFLAGS_WARN_ON -= -Wall

win32 {
    DEFINES += -Dinline=__inline
    INCLUDEPATH += C:\NugetPackages\OpenCV.2.4.10\build\native\include
    LIBS += -LC:\NugetPackages\OpenCV.2.4.10\build\native\lib\Win32\v120\Release \
        -lopencv_core2410 \
        -lopencv_highgui2410 \
        -lopencv_imgproc2410 \
        -lopencv_video2410
} else {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib \
        -lopencv_core \
        -lopencv_highgui \
        -lopencv_imgproc \
        -lopencv_video \
        #-lopencv_videoio \
        #-lopencv_imgcodecs
}

INCLUDEPATH += darknet/

SOURCES += main.cpp\
        mainwindow.cpp \
        fpscounter.cpp \
        framereader.cpp \
    darknet/activation_layer.cpp \
    darknet/activations.cpp \
    darknet/avgpool_layer.cpp \
    darknet/batchnorm_layer.cpp \
    darknet/blas.cpp \
    darknet/box.cpp \
    darknet/col2im.cpp \
    darknet/connected_layer.cpp \
    darknet/convolutional_layer.cpp \
    darknet/cost_layer.cpp \
    darknet/cpu_gemm.cpp \
    darknet/crnn_layer.cpp \
    darknet/crop_layer.cpp \
    darknet/darknet.cpp \
    darknet/data.cpp \
    darknet/deconvolutional_layer.cpp \
    darknet/demo.cpp \
    darknet/detection_layer.cpp \
    darknet/dropout_layer.cpp \
    darknet/gemm.cpp \
    darknet/gru_layer.cpp \
    darknet/im2col.cpp \
    darknet/image.cpp \
    darknet/layer.cpp \
    darknet/list.cpp \
    darknet/local_layer.cpp \
    darknet/matrix.cpp \
    darknet/maxpool_layer.cpp \
    darknet/network.cpp \
    darknet/normalization_layer.cpp \
    darknet/option_list.cpp \
    darknet/parser.cpp \
    darknet/rnn_layer.cpp \
    darknet/rnn_vid.cpp \
    darknet/route_layer.cpp \
    darknet/shortcut_layer.cpp \
    darknet/softmax_layer.cpp \
    darknet/utils.cpp \
    darknet/xnor_layer.cpp \
    darknet/yolo.cpp \
    darknet/cuda.cpp

HEADERS  += mainwindow.h \
    fpscounter.h \
    framereader.h \
    timer.h \
    darknet/activation_layer.h \
    darknet/activations.h \
    darknet/avgpool_layer.h \
    darknet/batchnorm_layer.h \
    darknet/blas.h \
    darknet/box.h \
    darknet/classifier.h \
    darknet/col2im.h \
    darknet/connected_layer.h \
    darknet/convolutional_layer.h \
    darknet/cost_layer.h \
    darknet/crnn_layer.h \
    darknet/crop_layer.h \
    darknet/data.h \
    darknet/deconvolutional_layer.h \
    darknet/demo.h \
    darknet/detection_layer.h \
    darknet/dropout_layer.h \
    darknet/gemm.h \
    darknet/gru_layer.h \
    darknet/im2col.h \
    darknet/image.h \
    darknet/layer.h \
    darknet/list.h \
    darknet/local_layer.h \
    darknet/matrix.h \
    darknet/maxpool_layer.h \
    darknet/network.h \
    darknet/normalization_layer.h \
    darknet/option_list.h \
    darknet/parser.h \
    darknet/rnn_layer.h \
    darknet/route_layer.h \
    darknet/shortcut_layer.h \
    darknet/softmax_layer.h \
    darknet/stb_image.h \
    darknet/stb_image_write.h \
    darknet/utils.h \
    darknet/xnor_layer.h \
    darknet/cuda.h

FORMS    += mainwindow.ui

RESOURCES +=

RC_FILE += L2.rc

RESOURCES += \
    style.qrc

win32 {
RC_FILE += \
    app.rc
}
