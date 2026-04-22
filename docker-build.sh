#!/bin/bash
#
# Local Docker build script for WiiBrowser-Lite
# Uses the official devkitPPC Docker container
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}WiiBrowser-Lite Docker Build Script${NC}"
echo "====================================="
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo -e "${RED}Error: Docker is not installed${NC}"
    echo "Please install Docker from https://docs.docker.com/get-docker/"
    exit 1
fi

# Check if Docker daemon is running
if ! docker info &> /dev/null; then
    echo -e "${RED}Error: Docker daemon is not running${NC}"
    echo "Please start Docker and try again"
    exit 1
fi

echo -e "${YELLOW}Pulling latest devkitPPC Docker image...${NC}"
docker pull devkitpro/devkitppc:latest

echo ""
echo -e "${YELLOW}Building WiiBrowser-Lite...${NC}"
echo ""

# Run the build in Docker container
# Mount current directory to /project in container
# Run make with all available cores
docker run --rm \
    -v "$(pwd):/project" \
    -w /project \
    devkitpro/devkitppc:latest \
    bash -c "make clean && make -j\$(nproc)"

echo ""
if [ -f "wiibrowserlite.dol" ]; then
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo ""
    echo "Build artifacts:"
    ls -lh wiibrowserlite.dol wiibrowserlite.elf 2>/dev/null || true
    echo ""
    echo "Binary size:"
    du -h wiibrowserlite.dol
    echo ""
    echo -e "${GREEN}You can now copy wiibrowserlite.dol to your Wii SD card${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi
