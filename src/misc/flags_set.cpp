#include "flags_set.h"

void setDisconnected() {
    disconnected = 1;
}

void unsetDisconnected() {
    disconnected = 0;
}

void setShutdownSIG() {
    shutdownSIG = 1;
}

void unsetShutdownSIG() {
    shutdownSIG = 0;
}
