# Nintendo DSi platform build configuration (stub)
#
# Target: Nintendo DSi (ARM9 @ 133MHz, 16MB RAM)
# Toolchain: devkitARM + libnds (DSi mode)
# Status: PLACEHOLDER - not yet implemented
#
# Constraints: HTTPS yes (mbedTLS fits), JavaScript MAYBE
# (QuickJS uses ~1MB - leaves ~14MB for DOM/network/UI).

$(error "DSi platform not yet implemented. Browser core needs platform/dsi/ port.")

# When implemented:
#   DEVKITARM ?= $(DEVKITPRO)/devkitARM
#   include $(DEVKITARM)/dsi_rules
#   WBL_HAS_JAVASCRIPT := 1
#   WBL_HAS_HTTPS      := 1
#   TARGET_OUT := $(TARGET).dsi
