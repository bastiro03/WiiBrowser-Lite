#ifndef WIIBROWSER_PNG_H
#define WIIBROWSER_PNG_H
#include <gccore.h>
#include <stdio.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
u8 *ReadPNG(const u8 *source, size_t length, unsigned *width, unsigned *height);
int WritePNG(FILE *file, unsigned width, unsigned height, const u8 *tiledTexture);
#ifdef __cplusplus
}
#endif
#endif
