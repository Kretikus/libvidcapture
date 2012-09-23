include(../config.pri)

TEMPLATE = lib
CONFIG  += staticlib
TARGET   = vidcapture
DESTDIR  = $${LIB_DIR}

DEPENDPATH  += .
INCLUDEPATH += .

CONFIG += console

HEADERS += \
	vidcapture.h \
	util.h \

SOURCES += \
	util.cpp \
	vidcapture.cpp \

win32 {
	HEADERS += ds_videodevice.h ds_videocapture.h ds_utils.h
	SOURCES += ds_videodevice.cpp ds_videocapture.cpp
}

unix:!macx {
	HEADERS += v4l_device.h
	SOURCES += v4l_device.cpp
}

macx {
	HEADERS           += mac_capture.h
	OBJECTIVE_SOURCES += mac_capture.mm
}
