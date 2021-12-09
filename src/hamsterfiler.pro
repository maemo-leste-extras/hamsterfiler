QT += core gui maemo5 dbus

TARGET = hamsterfiler
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingswindow.cpp \
    clipboardwindow.cpp \
    bookmarkswindow.cpp \
    applicationswindow.cpp \
    operationswindow.cpp \
    filedetailsdialog.cpp \
    attentiondialog.cpp \
    locationwidget.cpp \
    operationsprogressindicator.cpp \
    filelistview.cpp \
    rotator.cpp \
    clipboard.cpp \
    history.cpp \
    sizecounter.cpp \
    fileoperation.cpp \
    operationmanager.cpp \
    filesystemmodel.cpp \
    filesystemmodelworker.cpp \
    thumbnailjob.cpp\
    sharedialog.cpp \
    applicationdialog.cpp \
    createdialog.cpp

HEADERS += \
    global.h \
    mainwindow.h \
    settingswindow.h \
    clipboardwindow.h \
    bookmarkswindow.h \
    applicationswindow.h \
    operationswindow.h \
    filedetailsdialog.h \
    attentiondialog.h \
    confirmdialog.h \
    locationwidget.h \
    operationsprogressindicator.h \
    filelistview.h \
    filesystemmodel.h \
    thumbnailjob.h \
    rotator.h \
    clipboard.h \
    history.h \
    sizecounter.h \
    fileoperation.h \
    operationmanager.h \
    operationdelegate.h \
    detaileddelegate.h \
    griddelegate.h \
    descriptivedelegate.h \
    sharedialog.h \
    applicationdialog.h \
    prefixvalidator.h \
    createdialog.h

FORMS += \
    mainwindow.ui \
    settingswindow.ui \
    clipboardwindow.ui \
    bookmarkswindow.ui \
    applicationswindow.ui \
    operationswindow.ui \
    filedetailsdialog.ui \
    attentiondialog.ui

isEmpty(PREFIX) {
    PREFIX = /usr
}

BINDIR = /usr/bin
DATADIR = /usr/share
OPTDIR = /opt/$${TARGET}

DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

INSTALLS += target launcher desktop icon64

target.path = $$OPTDIR

launcher.path = $$BINDIR
launcher.files = ../extra/$${TARGET}

desktop.path = $$DATADIR/applications/hildon
desktop.files += ../extra/$${TARGET}.desktop

icon64.path = $$DATADIR/icons/hicolor/64x64/apps
icon64.files += ../extra/$${TARGET}.png

LIBS += -lhildonthumbnail
CONFIG += link_pkgconfig
PKGCONFIG += gtk+-2.0 glib-2.0 libhildonmime
