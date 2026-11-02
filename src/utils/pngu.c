/* WiiBrowser PNG adapter. libpng's bounded, recoverable simplified API replaces
 * the legacy length-less PNGU reader. No third-party decoder is copied here.
 */
#include <png.h>
#include <stdlib.h>
#include <string.h>
#include "pngu.h"

#if !defined(PNG_SIMPLIFIED_READ_SUPPORTED) || !defined(PNG_SIMPLIFIED_WRITE_SUPPORTED)
#error WiiBrowser requires libpng 1.6 with simplified read/write support
#endif

static unsigned read32(const u8 *bytes)
{
    return ((unsigned)bytes[0] << 24) | ((unsigned)bytes[1] << 16) |
           ((unsigned)bytes[2] << 8) | bytes[3];
}

static int preflight(const u8 *source, size_t length)
{
    if (length < 33 || memcmp(source, "\x89PNG\r\n\x1a\n", 8) ||
        read32(source + 8) != 13 || memcmp(source + 12, "IHDR", 4)) return 0;
    const unsigned width = read32(source + 16), height = read32(source + 20);
    if (!width || !height || width > 4096 || height > 4096 || width > (1024 * 1024) / height) return 0;
    size_t cursor = 8, chunks = 0;
    int dataSeen = 0;
    while (cursor <= length - 12) {
        if (++chunks > 4096) return 0;
        const unsigned chunk = read32(source + cursor);
        const u8 *type = source + cursor + 4;
        if (chunk > length - cursor - 12) return 0;
        // Avoid compressed ancillary metadata allocating a large hidden working
        // set before the pixel budget can be applied. Image data remains capped.
        if (!memcmp(type, "iCCP", 4) || !memcmp(type, "zTXt", 4) || !memcmp(type, "iTXt", 4)) return 0;
        if (!memcmp(type, "IDAT", 4)) dataSeen = 1;
        else if (chunk > 64 * 1024) return 0;
        if (!memcmp(type, "IEND", 4)) return dataSeen && !chunk && cursor + 12 == length;
        cursor += (size_t)chunk + 12;
    }
    return 0;
}

u8 *ReadPNG(const u8 *source, size_t length, unsigned *width, unsigned *height)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    if (!source || !width || !height || length > 4 * 1024 * 1024 || !preflight(source, length)) return NULL;
    if (!png_image_begin_read_from_memory(&image, source, length)) { png_image_free(&image); return NULL; }
    if (!image.width || !image.height || image.width > 4096 || image.height > 4096 ||
        image.width > (1024 * 1024) / image.height) {
        png_image_free(&image);
        return NULL;
    }
    image.format = PNG_FORMAT_RGBA;
    u8 *pixels = (u8 *)malloc(PNG_IMAGE_SIZE(image));
    if (!pixels) { png_image_free(&image); return NULL; }
    if (!png_image_finish_read(&image, NULL, pixels, 0, NULL)) {
        free(pixels); png_image_free(&image); return NULL;
    }
    *width = image.width;
    *height = image.height;
    png_image_free(&image);
    return pixels;
}

int WritePNG(FILE *file, unsigned width, unsigned height, const u8 *texture)
{
    if (!file || !texture || !width || !height || width > 1024 || height > 1024 ||
        width % 4 || height % 4) return 0;
    u8 *pixels = (u8 *)malloc((size_t)width * height * 4);
    if (!pixels) return 0;
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            const size_t offset = ((((y >> 2) * (width >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
            u8 *pixel = pixels + ((size_t)y * width + x) * 4;
            pixel[0] = texture[offset + 1];
            pixel[1] = texture[offset + 32];
            pixel[2] = texture[offset + 33];
            pixel[3] = texture[offset];
        }
    }
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    image.width = width;
    image.height = height;
    image.format = PNG_FORMAT_RGBA;
    const int result = png_image_write_to_stdio(&image, file, 0, pixels, 0, NULL);
    png_image_free(&image);
    free(pixels);
    return result;
}
