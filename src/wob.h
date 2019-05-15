#ifndef __LOADCELL__H__
#define __LOADCELL__H__

#include "include/pybind11/embed.h"

typedef struct wob {
    pybind11::object HX711;
} wob;

typedef wob* wob_t;

wob_t makeWOB();
float readWOB(wob_t loadcell);
void freeWOB(wob_t loadcell);

#endif
