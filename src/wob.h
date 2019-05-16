#ifndef __LOADCELL__H__
#define __LOADCELL__H__

#include "include/pybind11/embed.h"

typedef struct wob {
    pybind11::object HX711;
} wob;

typedef wob* wob_t;

wob_t make_wob();
float read_wob(wob_t loadcell);
void free_wob(wob_t loadcell);

#endif
