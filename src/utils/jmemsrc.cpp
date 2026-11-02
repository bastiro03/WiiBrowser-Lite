/* WiiBrowser JPEG adapter: libjpeg memory source, recoverable errors, bounded
 * dimensions. Replaces the legacy fatal-error/scaler glue.
 */
#include <cstdio>
#include <cstdlib>
#include <csetjmp>
#include <jpeglib.h>
#include "jmemsrc.h"
#include "texture.h"

struct DecodeError { jpeg_error_mgr base; jmp_buf jump; };
struct DecodeState { jpeg_decompress_struct info; DecodeError error; u8 *pixels; };
static void failJPEG(j_common_ptr common)
{
    DecodeError *error = reinterpret_cast<DecodeError *>(common->err);
    longjmp(error->jump, 1);
}

u8 *DecodeJPEG(const u8 *source, u32 length, int *width, int *height, u8 *previous)
{
    (void)previous;
    if (!source || length < 2 || length > wbl::MaxImageBytes) return NULL;
    DecodeState *state = static_cast<DecodeState *>(calloc(1, sizeof(DecodeState)));
    if (!state) return NULL;
    state->info.err = jpeg_std_error(&state->error.base);
    state->error.base.error_exit = failJPEG;
    if (setjmp(state->error.jump)) {
        jpeg_destroy_decompress(&state->info);
        free(state->pixels); free(state); return NULL;
    }
    jpeg_create_decompress(&state->info);
    jpeg_mem_src(&state->info, source, length);
    jpeg_read_header(&state->info, TRUE);
    size_t bytes;
    if (!wbl::imageSize(state->info.image_width, state->info.image_height, 4, bytes)) {
        jpeg_destroy_decompress(&state->info); free(state); return NULL;
    }
    state->info.out_color_space = JCS_RGB;
    state->info.mem->max_memory_to_use = wbl::MaxImageBytes;
    for (unsigned denominator = 1; denominator <= 8; denominator *= 2) {
        state->info.scale_num = 1;
        state->info.scale_denom = denominator;
        jpeg_calc_output_dimensions(&state->info);
        if (state->info.output_width <= 768 && state->info.output_height <= 480) break;
    }
    jpeg_start_decompress(&state->info);
    if (state->info.output_components != 3 ||
        !wbl::imageSize(state->info.output_width, state->info.output_height, 3, bytes)) {
        jpeg_destroy_decompress(&state->info); free(state); return NULL;
    }
    state->pixels = static_cast<u8 *>(malloc(bytes));
    if (!state->pixels) { jpeg_destroy_decompress(&state->info); free(state); return NULL; }
    while (state->info.output_scanline < state->info.output_height) {
        JSAMPROW row = state->pixels + static_cast<size_t>(state->info.output_scanline) * state->info.output_width * 3;
        if (jpeg_read_scanlines(&state->info, &row, 1) != 1) failJPEG(reinterpret_cast<j_common_ptr>(&state->info));
    }
    jpeg_finish_decompress(&state->info);
    u8 *texture = TilePixels(state->pixels, state->info.output_width, state->info.output_height, 3, width, height);
    jpeg_destroy_decompress(&state->info);
    free(state->pixels); free(state);
    return texture;
}
