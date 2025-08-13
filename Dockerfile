# Simple Dockerfile for manual build and run
FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install essential packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc \
    g++ \
    make \
    git \
    gdb \
    valgrind \
    libgtest-dev \
    vim \
    nano \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Build and install Google Test
RUN cd /usr/src/gtest && \
    cmake . && \
    make && \
    cp lib/*.a /usr/lib/

# Create user with same UID as typical macOS user
RUN useradd -m -u 1000 -s /bin/bash developer

# Set working directory
WORKDIR /workspace

# Switch to developer user
USER developer

# Default command - interactive bash shell
CMD ["/bin/bash"]