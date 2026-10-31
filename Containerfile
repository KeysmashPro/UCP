FROM docker.io/library/debian:trixie-slim

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    clang \
    libvulkan-dev \
    glslc \
    libglfw3 \
    libglfw3-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxxf86vm-dev \
    pkg-config \
    gosu \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
