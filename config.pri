PROJ_ROOT=$$PWD

exists($${PROJ_ROOT}/local.pri) {
	include($${PROJ_ROOT}/local.pri)
} else {
	error("note: local config file (local.pri) not available")
}

CONFIG(debug) {
	CONFIG -= debug release
	CONFIG += debug
	DEBUG_OR_RELEASE = debug
} else {
	CONFIG -= debug release
	CONFIG += release
	DEBUG_OR_RELEASE = release
	DEFINES += NDEBUG
}

##############################################################################
# directories
##############################################################################

# generated libraries
LIB_DIR       = $${PROJ_ROOT}/libs/$${DEBUG_OR_RELEASE}

# generated binaries
BIN_DIR      = $${PROJ_ROOT}/bin/$${DEBUG_OR_RELEASE}

# complied objects
OBJECTS_DIR  = build/$${DEBUG_OR_RELEASE}

# generated files
MOC_DIR      = build/gen/moc
UI_DIR       = build/gen/ui

win32 {
	LIB_PREFIX  =
	LIB_POSTFIX = lib
	EXE_POSTFIX = .exe
} else {
	LIB_PREFIX  = lib
	LIB_POSTFIX = a
	EXE_POSTFIX =
}

##############################################################################
# compiler configuration
##############################################################################
INCLUDEPATH += $${PROJ_ROOT} build/gen .
DEPENDPATH  += $${PROJ_ROOT}
win32 {
		QMAKE_CXXFLAGS += /Fdqmake.pdb
} else {
		QMAKE_CXXFLAGS += -Wall
	!macx {
		QMAKE_CXXFLAGS += --std=c++11
	}
}

##############################################################################
# linker configuration
##############################################################################
QMAKE_LIBDIR += $${LIB_DIR}
DESTDIR      = $${BIN_DIR}
unix:!macx {
	EXT_LIBS += -lv4l2
}

macx {
	EXT_LIBS += -lobjc -framework cocoa -framework QTKit -framework QuartzCore
}


##############################################################################
# helper
##############################################################################
defineTest(initApp) {
	TEMPLATE = app

	for(lib, 1) {
		LIBS += -l$${lib}
		POST_TARGETDEPS += $${LIB_DIR}/$${LIB_PREFIX}$${lib}.$${LIB_POSTFIX}
	}
	LIBS += $$EXT_LIBS

	export(TEMPLATE)
	export(LIBS)
	export(POST_TARGETDEPS)

#		!win32:release:!no_strip:console {
#			QMAKE_POST_LINK += strip $${DESTDIR}/$${TARGET}
#			export(QMAKE_POST_LINK)
#		}

	macx:console {
		CONFIG -= app_bundle
		export(CONFIG)
	}
}

