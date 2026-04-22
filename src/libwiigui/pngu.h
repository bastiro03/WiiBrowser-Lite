/********************************************************************************************
*
* PNGU (libwiigui shim)
*
* The canonical pngu implementation now lives in src/utils/pngu.[ch]. This
* header used to carry a duplicate (and drifted) declaration of DecodePNG
* that didn't match the implementation, which caused undefined references
* at link time under LTO.
*
* Forward to the real header so every translation unit sees one
* declaration + one definition.
*
* NOTE: we intentionally do NOT wrap the include in our own header guard,
* because utils/pngu.h uses the same `#ifndef __PNGU__` sentinel. A guard
* here would set __PNGU__ before the real header runs, short-circuiting
* it and leaving DecodePNG undeclared.
*
********************************************************************************************/

#include "../utils/pngu.h"
