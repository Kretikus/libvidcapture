include(../config.pri)

QT += testlib

HEADERS += \
	testutil.h

SOURCES += \
	testutil.cpp \
	testmain.cpp \
	utiltest.cpp \

initApp(vidcapture)
