TEMPLATE	= app
CONFIG		+= qt release
QT		+= core gui network
greaterThan(QT_MAJOR_VERSION, 4) {
QT += widgets
}

HEADERS		+= qcounter.h
SOURCES		+= qcounter.cpp
RESOURCES	+= qcounter.qrc

macx {
TARGET		= QuickCounter
} else {
TARGET		= qcounter
}

isEmpty(PREFIX) {
PREFIX          = /usr/local
}

win32 {
QMAKE_LFLAGS	+= -static-libgcc -static
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

macx {
icons.path= $${DESTDIR}/Contents/Resources
icons.files= qaudiosonar.icns
QMAKE_BUNDLE_DATA+= icons
} else {
icons.path	= $${PREFIX}/share/pixmaps
icons.files	= qcounter_48x48.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= qcounter.desktop
INSTALLS	+= desktop
}
