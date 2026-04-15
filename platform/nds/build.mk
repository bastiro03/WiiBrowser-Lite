# Nintendo DS platform build configuration (stub)
#
# Target: Nintendo DS (ARM9 @ 67MHz, 4MB RAM)
# Toolchain: devkitARM + libnds
# Status: PLACEHOLDER - not yet implemented
#
# Constraints: no JavaScript, no HTTPS (TLS too heavy for 4MB),
# CSS1 only, no archive support, minimal image formats.

$(error "NDS platform not yet implemented. Browser core needs platform/nds/ port.")

# When implemented:
#   DEVKITARM ?= $(DEVKITPRO)/devkitARM
#   include $(DEVKITARM)/ds_rules
#   WBL_HAS_JAVASCRIPT := 0
#   WBL_HAS_HTTPS      := 0
#   TARGET_OUT := $(TARGET).nds
