NAME         = QtUCL
TARGET       = $${NAME}
QT           = core
QT          -= gui
CONFIG(static,static|shared) {
# static version does not support Qt Script now
QT          -= script
} else {
QT          += script
}

load(qt_build_config)
load(qt_module)

INCLUDEPATH += $${PWD}/../../include/QtUCL
INCLUDEPATH += $${PWD}/../../include/QtUCL/ucl

HEADERS     += $${PWD}/../../include/QtUCL/QtUCL
HEADERS     += $${PWD}/../../include/QtUCL/qtucl.h

SOURCES     += $${PWD}/qtucl.cpp
SOURCES     += $${PWD}/ScriptableUCL.cpp

OTHER_FILES += $${PWD}/../../include/$${NAME}/ucl/*
OTHER_FILES += $${PWD}/../../include/$${NAME}/headers.pri

include ($${PWD}/../../doc/Qt/Qt.pri)

win32 {

CONFIG(debug, debug|release) {
LIBS        += -lucl
} else {
LIBS        += -lucl
}

}

macx {

CONFIG(debug, debug|release) {
LIBS        += -llibucld
} else {
LIBS        += -llibucl
}

}

unix {

CONFIG(debug, debug|release) {
LIBS        += -llibucld
} else {
LIBS        += -llibucl
}

}
