#ifndef WIIBROWSER_TEXTURE_H
#define WIIBROWSER_TEXTURE_H
#include <gccore.h>
#include <malloc.h>
#include <string.h>
#include <algorithm>
#include "safety.h"

// CPU pixels → newly allocated, padded, aligned GX RGBA8 tiles. Never reuse an
// existing texture allocation without knowing its capacity.
inline u8 *TilePixels(const u8 *source, unsigned width, unsigned height,
                     unsigned channels, int *outWidth, int *outHeight)
{
    size_t bytes;
    if (!source || !outWidth || !outHeight || (channels != 3 && channels != 4) ||
        !wbl::imageSize(width, height, channels, bytes)) return NULL;
    unsigned targetWidth = width, targetHeight = height;
    if (targetWidth > 768 || targetHeight > 480) {
        const double factor = std::min(768.0 / width, 480.0 / height);
        targetWidth = std::max(1u, static_cast<unsigned>(width * factor));
        targetHeight = std::max(1u, static_cast<unsigned>(height * factor));
    }
    const unsigned paddedWidth = (targetWidth + 3) & ~3u;
    const unsigned paddedHeight = (targetHeight + 3) & ~3u;
    const size_t length = static_cast<size_t>(paddedWidth) * paddedHeight * 4;
    u8 *destination = static_cast<u8 *>(memalign(32, length));
    if (!destination) return NULL;
    memset(destination, 0, length);
    for (unsigned y = 0; y < targetHeight; ++y) {
        for (unsigned x = 0; x < targetWidth; ++x) {
            const unsigned sx = static_cast<unsigned long long>(x) * width / targetWidth;
            const unsigned sy = static_cast<unsigned long long>(y) * height / targetHeight;
            const u8 *pixel = source + (static_cast<size_t>(sy) * width + sx) * channels;
            const size_t offset = ((((y >> 2) * (paddedWidth >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
            destination[offset] = channels == 4 ? pixel[3] : 255;
            destination[offset + 1] = pixel[0];
            destination[offset + 32] = pixel[1];
            destination[offset + 33] = pixel[2];
        }
    }
    *outWidth = paddedWidth;
    *outHeight = paddedHeight;
    DCFlushRange(destination, length);
    return destination;
}
#endif
