######################################################################
# Automatically generated by qmake (2.01a) Fri Jan 4 19:08:59 2013
######################################################################

TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
QT += network

win32 {
	CONFIG -= debug_and_release
	CONFIG += release
	allstatic {
		QMAKE_LFLAGS += -static -static-libgcc
	}
}

HEADERS += pr0nmain.h picwidget.h savepicdlg.h qlabcolor.h

FORMS += pr0nmain.ui picwidget.ui savepicdlg.ui

SOURCES += main.cpp pr0nmain.cpp picwidget.cpp savepicdlg.cpp qlabcolor.cpp

