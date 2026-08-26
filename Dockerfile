FROM devkitpro/devkitppc:latest

# Install portlibs and tools needed for WiiBrowser Lite
# Q3: migrated from vendored libs/ to dkp-pacman
RUN dkp-pacman -Sy --noconfirm && \
    dkp-pacman -S --noconfirm \
        devkitPPC \
        libogc \
        ppc-zlib \
        ppc-libpng \
        ppc-freetype \
        ppc-libjpeg-turbo \
        ppc-curl \
        ppc-mxml \
        ppc-libvorbisidec \
        ppc-fribidi \
        ppc-ntfs \
        ppc-ext2fs \
        ppc-libfat \
        zip \
        flex \
        bison && \
    dkp-pacman -Sc --noconfirm

# Workdir matches CI
WORKDIR /src

# Verify toolchain
RUN powerpc-eabi-gcc --version && \
    ls -la /opt/devkitpro/portlibs/ppc/include/ | head

CMD ["make"]
