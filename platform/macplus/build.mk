# Apple Mac Plus platform build configuration (stub)
#
# Target: Apple Macintosh Plus (Motorola 68000 @ 8MHz, 1-4MB RAM)
# Toolchain: Retro68 (m68k-apple-macos cross-compiler)
# Status: PLACEHOLDER - not yet implemented
#
# Constraints: no JavaScript, no HTTPS (1MB RAM cannot fit TLS state),
# CSS1 only, no images larger than 256x256, GIF only,
# all networking via plain HTTP through a proxy gateway.

$(error "Mac Plus platform not yet implemented. Needs Retro68 toolchain + System 6 UI port.")

# When implemented:
#   include $(RETRO68)/Makefile.macplus
#   WBL_HAS_JAVASCRIPT := 0
#   WBL_HAS_HTTPS      := 0
#   WBL_HAS_JPEG       := 0
#   TARGET_OUT := $(TARGET).bin
