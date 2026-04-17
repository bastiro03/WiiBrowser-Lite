/* test_freetype_init.cpp - Unit test for FreeTypeGX initialization bug
 *
 * Bug: fontSystem[MAX_FONT_SIZE+1] array is only zeroed up to index 49,
 * leaving slots 50-100 with uninitialized garbage. When code checks
 * `if(!fontSystem[size])` for size >= 50, the garbage appears non-NULL,
 * skips allocation, and dereferences → crash with corrupted this pointer.
 *
 * Reproduction:
 * 1. Call InitFreeType()
 * 2. Request font size >= 50 (e.g., 83 bytes)
 * 3. Without the fix: fontSystem[83] contains garbage (e.g., 0xC)
 * 4. Code skips `new FreeTypeGX(83)`, tries to use garbage pointer
 * 5. Crash in RB-tree::find() with this=0xC, DAR=0x14
 *
 * Expected behavior with fix:
 * - All slots 0-100 initialized to NULL
 * - First use of size 83 allocates new FreeTypeGX
 * - No crash
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_FONT_SIZE 100

// Minimal mock of the fontSystem array
void* fontSystem[MAX_FONT_SIZE + 1];

void InitFreeType_BUGGY() {
    // BUG: only zeros 0-49, leaves 50-100 uninitialized
    for (int i = 0; i < 50; i++)
        fontSystem[i] = NULL;
}

void InitFreeType_FIXED() {
    // FIXED: zero entire array
    for (int i = 0; i <= MAX_FONT_SIZE; i++)
        fontSystem[i] = NULL;
}

int main() {
    printf("Testing FreeTypeGX initialization bug...\n\n");

    // Test 1: BUGGY version leaves high indexes uninitialized
    printf("Test 1: BUGGY InitFreeType\n");
    // Poison the array with non-NULL garbage
    memset(fontSystem, 0xAB, sizeof(fontSystem));

    InitFreeType_BUGGY();

    // Slots 0-49 should be NULL
    for (int i = 0; i < 50; i++) {
        if (fontSystem[i] != NULL) {
            fprintf(stderr, "FAIL: fontSystem[%d] = %p (expected NULL)\n", i, fontSystem[i]);
            return 1;
        }
    }
    printf("  ✓ Slots 0-49 correctly NULL\n");

    // Slots 50-100 should ALSO be NULL but aren't (BUG!)
    int uninitialized_count = 0;
    for (int i = 50; i <= MAX_FONT_SIZE; i++) {
        if (fontSystem[i] != NULL) {
            uninitialized_count++;
            if (uninitialized_count == 1) {
                fprintf(stderr, "  ✗ BUG DETECTED: fontSystem[%d] = %p (garbage, not NULL)\n",
                        i, fontSystem[i]);
            }
        }
    }

    if (uninitialized_count > 0) {
        printf("  ✗ %d/%d high slots contain garbage (would cause crash on access)\n",
               uninitialized_count, MAX_FONT_SIZE - 49);
    } else {
        fprintf(stderr, "UNEXPECTED: buggy version zeroed all slots (test setup error?)\n");
        return 1;
    }

    // Test 2: FIXED version zeros entire array
    printf("\nTest 2: FIXED InitFreeType\n");
    memset(fontSystem, 0xCD, sizeof(fontSystem));  // Different poison

    InitFreeType_FIXED();

    for (int i = 0; i <= MAX_FONT_SIZE; i++) {
        if (fontSystem[i] != NULL) {
            fprintf(stderr, "FAIL: fontSystem[%d] = %p after FIXED init\n", i, fontSystem[i]);
            return 1;
        }
    }
    printf("  ✓ All slots 0-100 correctly NULL\n");

    // Test 3: Simulate real-world access pattern for size 83
    printf("\nTest 3: Simulated access to fontSystem[83]\n");

    // Setup buggy state
    memset(fontSystem, 0x0C, sizeof(fontSystem));  // Poison with 0x0C (matches crash r3=0xC!)
    InitFreeType_BUGGY();

    int size = 83;
    void* ptr = fontSystem[size];

    if (ptr == NULL) {
        printf("  ✗ UNEXPECTED: fontSystem[83] is NULL (bug not reproduced)\n");
        return 1;
    } else {
        printf("  ✓ BUG REPRODUCED: fontSystem[83] = %p (garbage)\n", ptr);
        printf("    Code would skip allocation and dereference %p → CRASH\n", ptr);

        // This is what the real code does:
        // if (!fontSystem[83])  ← FALSE because ptr is non-NULL garbage
        //     fontSystem[83] = new FreeTypeGX(83);  ← SKIPPED
        // return fontSystem[83]->getWidth(text);  ← Dereference 0xC → DSI at 0x14
    }

    printf("\n=== ALL TESTS PASSED ===\n");
    printf("Buggy version: detected uninitialized array slots 50-100\n");
    printf("Fixed version: all slots correctly initialized to NULL\n");
    printf("Apply fix to FreeTypeGX.cpp lines 37, 55\n");

    return 0;
}
