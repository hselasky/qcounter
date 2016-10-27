TEMPLATE	= app
CONFIG		+= qt release
QT		+= core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
}

HEADERS		+= qcounter.h
SOURCES		+= qcounter.cpp
RESOURCES	+= qcounter.qrc

TARGET		= qcounter

win32 {
QMAKE_LFLAGS	+= -static-libgcc -static
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

icons.path	= $${PREFIX}/share/pixmaps
icons.files	= qcounter.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= qcounter.desktop
INSTALLS	+= desktop
