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
    DEPLOY_COMMAND = windeployqt

    CONFIG( debug, debug|release ) {
	DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$.exe))
    } else {
	DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$.exe))
    }
    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}

target.path	= $${PREFIX}/bin
INSTALLS	+= target

icons.path	= $${PREFIX}/share/pixmaps
icons.files	= qcounter.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= qcounter.desktop
INSTALLS	+= desktop
