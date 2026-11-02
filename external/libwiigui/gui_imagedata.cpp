/* libwiigui / WiiBrowser integration; Tantric 2009, modified by gave92.
 * Bounded image integration and explicit replacement ownership, 2026.
 */
#include "gui.h"
#include "gui_lock.h"
#include "embedded_assets.h"
#include "utils/pngu.h"
#include "utils/jmemsrc.h"
#include "utils/texture.h"

GuiImageData::GuiImageData(const u8 *source)
    : data(NULL), width(0), height(0), format(GX_TF_RGBA8)
{
    const size_t length = EmbeddedPngSize(source);
    if (length) LoadPNG(source, length);
}
GuiImageData::GuiImageData(const u8 *source, int length, u8 textureFormat)
    : data(NULL), width(0), height(0), format(textureFormat)
{
    SetImage(source, length);
}
GuiImageData::~GuiImageData() { free(data); }
void GuiImageData::SetData(u8 *source) { GuiScopedLock lock; if (data != source) free(data); data = source; }
void GuiImageData::SetImage(const u8 *source, int length)
{
    if (!source || length < 8 || static_cast<size_t>(length) > wbl::MaxImageBytes) return;
    if (source[0] == 0xff && source[1] == 0xd8) LoadJPEG(source, length);
    else if (source[0] == 0x89 && source[1] == 'P' && source[2] == 'N' && source[3] == 'G') LoadPNG(source, length);
    // GIF/BMP remain quarantined until their legacy parsers pass safety tests.
}
void GuiImageData::LoadPNG(const u8 *source, size_t length)
{
    unsigned rawWidth = 0, rawHeight = 0;
    u8 *pixels = ReadPNG(source, length, &rawWidth, &rawHeight);
    int nextWidth = 0, nextHeight = 0;
    u8 *texture = pixels ? TilePixels(pixels, rawWidth, rawHeight, 4, &nextWidth, &nextHeight) : NULL;
    free(pixels);
    if (!texture) return;
    GuiScopedLock lock;
    free(data); data = texture; width = nextWidth; height = nextHeight;
}
void GuiImageData::LoadJPEG(const u8 *source, int length)
{
    int nextWidth = 0, nextHeight = 0;
    u8 *texture = DecodeJPEG(source, length, &nextWidth, &nextHeight, NULL);
    if (!texture) return;
    GuiScopedLock lock;
    free(data); data = texture; width = nextWidth; height = nextHeight;
}
void GuiImageData::LoadBMP(const u8 *, int) {}
void GuiImageData::LoadGIF(const u8 *, int) {}
u8 *GuiImageData::GetImage() { return data; }
int GuiImageData::GetWidth() { return width; }
int GuiImageData::GetHeight() { return height; }
u8 GuiImageData::GetFormat() { return format; }
