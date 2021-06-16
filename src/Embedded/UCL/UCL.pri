INCLUDEPATH += $${PWD}

HEADERS     += $${PWD}/qtucl.h

SOURCES     += $${PWD}/qtucl.cpp
SOURCES     += $${PWD}/ScriptableUCL.cpp

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
