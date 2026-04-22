/* test_guitext_init.cpp - Test for GuiText uninitialized member bugs
 *
 * Bug: GuiText constructor (gui_text.cpp:28-55) fails to initialize:
 *   - font: FreeTypeGX* pointer (line 755 in gui.h)
 *   - currentSize: int
 *
 * Consequence: When GuiLongText::Draw() executes:
 *   FreeTypeGX *ftgx = (font ? font : fontSystem[currentSize]);
 *   ftgx->drawText(...)
 *
 * If `font` is uninitialized non-zero garbage, ftgx = garbage, and
 * ftgx->drawText() dereferences the garbage pointer → DSI crash.
 *
 * Compile with MSan (clang only) to catch uninitialized reads, or with
 * ASan + UBsan to catch the downstream dereference.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Mock minimal layout of GuiText matching the real class.
 * All members declared in the same order as gui.h:741-755.
 */
typedef struct GuiText_BUGGY_ {
    // Base class stand-in (simplified)
    int base_parent;
    int base_visible;

    // Actual GuiText members (gui.h lines 741-755)
    struct { unsigned char r,g,b,a; } color;
    void* text;              // wchar_t*
    void* textDyn[20];       // wchar_t* array
    int textDynNum;
    char* origText;
    int size;
    int maxWidth;
    int textScroll;
    int textScrollPos;
    int textScrollInitialDelay;
    int textScrollDelay;
    unsigned short style;
    char wrap;
    int currentSize;          // ← UNINITIALIZED in constructor!
    void* font;               // ← UNINITIALIZED in constructor!
} GuiText_BUGGY;

void GuiText_BUGGY_ctor(GuiText_BUGGY* self, const char* t, int s) {
    // Mirrors gui_text.cpp:28-55
    self->origText = NULL;
    self->text = NULL;
    self->size = s;
    // color = c (skipped)
    // style = FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE
    self->style = 0x22;
    self->maxWidth = 0;
    self->wrap = 0;
    self->textDynNum = 0;
    self->textScroll = 0;
    self->textScrollPos = 0;
    self->textScrollInitialDelay = 0;
    self->textScrollDelay = 0;
    // alignmentHor, alignmentVert (skipped - in base class)
    for (int i = 0; i < 20; i++) self->textDyn[i] = NULL;

    // BUG: currentSize and font left uninitialized!
}

void GuiText_FIXED_ctor(GuiText_BUGGY* self, const char* t, int s) {
    GuiText_BUGGY_ctor(self, t, s);
    // FIX: initialize the two missed members
    self->currentSize = 0;
    self->font = NULL;
}

int main() {
    printf("Testing GuiText uninitialized member bug...\n\n");

    // Test 1: BUGGY constructor leaves font/currentSize uninitialized
    printf("Test 1: BUGGY GuiText constructor\n");

    /* Allocate on heap so ASan/MSan can track uninit reads.
     * Poison the memory first so uninitialised reads return non-zero. */
    GuiText_BUGGY* obj = (GuiText_BUGGY*)malloc(sizeof(GuiText_BUGGY));
    if (!obj) return 1;
    memset(obj, 0xAB, sizeof(*obj));  // Poison all members

    GuiText_BUGGY_ctor(obj, "hello", 20);

    // size should be initialized to 20
    if (obj->size != 20) {
        fprintf(stderr, "FAIL: size = %d (expected 20)\n", obj->size);
        free(obj);
        return 1;
    }
    printf("  ✓ size = 20 (correctly initialized)\n");

    // font should be NULL but isn't (BUG)
    if (obj->font == NULL) {
        fprintf(stderr, "UNEXPECTED: font is NULL (test memset broken?)\n");
        free(obj);
        return 1;
    }
    printf("  ✗ BUG DETECTED: font = %p (uninitialized garbage)\n", obj->font);

    // currentSize should be 0 but isn't (BUG)
    if (obj->currentSize == 0) {
        fprintf(stderr, "UNEXPECTED: currentSize is 0 (test memset broken?)\n");
        free(obj);
        return 1;
    }
    printf("  ✗ BUG DETECTED: currentSize = 0x%x (uninitialized garbage)\n",
           obj->currentSize);

    /* Simulate the GuiLongText code path:
     *   FreeTypeGX *ftgx = (font ? font : fontSystem[currentSize]);
     * If font is non-zero garbage, ftgx = garbage, and next dereference crashes.
     */
    void* simulated_ftgx = obj->font ? obj->font : (void*)0xDEADBEEF;
    if (simulated_ftgx == obj->font && obj->font != NULL) {
        printf("  ✗ GuiLongText path would dereference garbage pointer %p\n",
               simulated_ftgx);
        printf("    → DSI crash on real hardware\n");
    }
    free(obj);

    // Test 2: FIXED constructor initializes all members
    printf("\nTest 2: FIXED GuiText constructor\n");

    obj = (GuiText_BUGGY*)malloc(sizeof(GuiText_BUGGY));
    if (!obj) return 1;
    memset(obj, 0xCD, sizeof(*obj));  // Poison with different value

    GuiText_FIXED_ctor(obj, "hello", 20);

    if (obj->font != NULL) {
        fprintf(stderr, "FAIL: font = %p after fix (expected NULL)\n", obj->font);
        free(obj);
        return 1;
    }
    if (obj->currentSize != 0) {
        fprintf(stderr, "FAIL: currentSize = 0x%x after fix (expected 0)\n",
                obj->currentSize);
        free(obj);
        return 1;
    }
    printf("  ✓ font = NULL\n");
    printf("  ✓ currentSize = 0\n");

    free(obj);

    printf("\n=== TEST PASSED ===\n");
    printf("Bug reproduced: GuiText ctor leaves `font` and `currentSize` uninitialized.\n");
    printf("Fix: initialize both to safe defaults (NULL, 0) in constructor.\n");
    printf("Files to fix: src/libwiigui/gui_text.cpp (both constructors)\n");
    return 0;
}
