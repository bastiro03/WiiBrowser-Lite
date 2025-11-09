# Building WiiBrowser-Lite

This document describes how to build WiiBrowser-Lite with the latest devkitPPC toolchain, including Link Time Optimization (LTO) and modern HTTPS support.

## Requirements

### DevkitPro Toolchain

WiiBrowser-Lite requires the latest devkitPro toolchain for Wii development:

- **devkitPPC r47** or later (includes GCC 15.1.0)
- **libogc** - Official GameCube/Wii library
- **libfat** - FAT filesystem support
- Standard Wii development libraries

### Installation

#### Linux/macOS

1. Install the devkitPro package manager:
```bash
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
sudo ./install-devkitpro-pacman
```

2. Install the Wii development toolchain:
```bash
sudo dkp-pacman -S wii-dev
```

3. Set up environment variables (add to `~/.bashrc` or `~/.zshrc`):
```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=/opt/devkitpro/devkitPPC
export PATH=$DEVKITPPC/bin:$PATH
```

4. Reload your shell or run:
```bash
source ~/.bashrc
```

#### Windows

1. Download the latest graphical installer from:
   https://github.com/devkitPro/installer/releases

2. Run the installer and select "Wii Development" when prompted

3. Environment variables should be set automatically

## Build Features

### Link Time Optimization (LTO)

This build uses `-flto` to enable Link Time Optimization, which:
- Reduces binary size by eliminating dead code across compilation units
- Enables whole-program optimization
- Improves performance through better inlining decisions

The Makefile is configured with:
- `-Os`: Optimize for size
- `-flto`: Enable link-time optimization
- `-ffunction-sections -fdata-sections`: Separate functions/data into sections
- `-Wl,--gc-sections`: Remove unused sections at link time

### Modern HTTPS Support

The browser now includes:
- **Proper SSL/TLS certificate verification** (previously disabled)
- **Embedded Mozilla CA certificate bundle** (~226KB)
- **Support for modern HTTPS websites** including Wikipedia and GitHub
- **Automatic fallback** if certificate bundle cannot be written

The CA certificate bundle is automatically embedded in the binary and extracted
at runtime to the SD card for use by libcurl.

## Building

1. Clone the repository:
```bash
git clone https://github.com/matthargett/WiiBrowser-Lite.git
cd WiiBrowser-Lite
```

2. Build the project:
```bash
make clean
make -j$(nproc)
```

This will produce:
- `wiibrowserlite.elf` - Executable in ELF format
- `wiibrowserlite.dol` - Wii executable in DOL format
- `wiibrowserlite.map` - Linker map file

3. The `.dol` file can be run on:
   - Real Wii hardware (via Homebrew Channel)
   - Dolphin emulator
   - Other Wii homebrew loaders

## Dependencies

All required libraries are included in the `libs/wii/` directory:

- libcurl 8.13.0 - HTTP/HTTPS client (478KB)
- libcyassl (wolfSSL) - SSL/TLS library (195KB)
- libfreetype - Font rendering (1.0MB)
- libjpeg, libpng - Image support
- libz - Compression
- And many others...

These libraries should ideally be recompiled with `-flto` for maximum benefit,
but the existing pre-compiled versions will work.

## Troubleshooting

### Build Errors

**Error: `powerpc-eabi-gcc: command not found`**
- Ensure devkitPPC is installed and environment variables are set correctly
- Verify with: `echo $DEVKITPPC`

**Error: Cannot find `-logc` or other libraries**
- Install the full `wii-dev` package: `sudo dkp-pacman -S wii-dev`
- Verify installation: `ls $DEVKITPRO/libogc`

**LTO warnings during linking**
- Normal when mixing LTO-compiled code with non-LTO libraries
- Can be ignored safely

### Runtime Issues

**HTTPS verification fails**
- Ensure the browser has write access to the SD card
- The CA bundle is written to `<app_path>/cacert.pem`
- If SD write fails, it falls back to `/tmp/cacert.pem`

**Certificate errors on some sites**
- The CA bundle is from https://curl.se/ca/cacert.pem (Mozilla's bundle)
- Updated regularly but may not include very new CAs
- Sites with self-signed certificates will fail (expected behavior)

## Development

### Updating the CA Certificate Bundle

To update the Mozilla CA certificate bundle:

```bash
wget -O certs/cacert.pem https://curl.se/ca/cacert.pem
```

Then rebuild the project.

### Modifying Compiler Flags

The main compiler flags are in the `Makefile`:

```makefile
CFLAGS := -Os -Wall -mrvl -mcpu=750 -meabi -mhard-float -flto \
          -ffunction-sections -fdata-sections

LDFLAGS := -flto -Wl,-Map,$(TARGET).map -Wl,--gc-sections
```

For debugging, you can:
- Add `-g` for debug symbols
- Change `-Os` to `-O0` to disable optimization
- Remove `-flto` if it causes issues

### Rebuilding Dependencies with LTO

For maximum optimization, dependencies should be rebuilt with matching flags:

```bash
# Example for libcurl
CFLAGS="-Os -flto -ffunction-sections -fdata-sections" \
LDFLAGS="-flto -Wl,--gc-sections" \
./configure --host=powerpc-eabi --prefix=/opt/devkitpro/portlibs/ppc
make
make install
```

This requires each library's source code and may need patches for cross-compilation.

## Performance Notes

With LTO and size optimizations:
- Binary size is significantly reduced compared to `-O2` without LTO
- Runtime performance is generally better or equivalent
- Link times are longer due to whole-program analysis

## License

See LICENSE.md for project licensing information.

## Additional Resources

- [devkitPro Wiki](https://devkitpro.org/wiki)
- [WiiBrew](https://wiibrew.org/) - Wii homebrew documentation
- [libogc Documentation](https://libogc.devkitpro.org/)
- [Original WiiBrowser](https://gbatemp.net/threads/wii-browser.342634/)
