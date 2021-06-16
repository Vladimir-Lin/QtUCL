QT             = core
QT            -= gui
QT            += QtUCL

CONFIG(debug, debug|release) {
TARGET         = ucltoold
} else {
TARGET         = ucltool
}

CONFIG        += console

TEMPLATE       = app

SOURCES       += $${PWD}/ucltool.cpp

win32 {
RC_FILE        = $${PWD}/ucltool.rc
OTHER_FILES   += $${PWD}/ucltool.rc
OTHER_FILES   += $${PWD}/*.js
}
