NAME         = QtUCL
TARGET       = $${NAME}
QT           = core
QT          -= gui

load(qt_build_config)
load(qt_module)

INCLUDEPATH += $${PWD}

HEADERS     += $${PWD}/qtucl.h

SOURCES     += $${PWD}/qtucl.cpp

include ($${PWD}/../UCL/UCL.pri)
