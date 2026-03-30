# Use Debian 11 to match the workflow's runs-on environment
FROM debian:latest

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    libopencv-dev \
    libcamera-dev \
    dos2unix \
    libgpiod-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy the project files into the container
COPY . .

# Convert all files from DOS to Unix line endings
RUN find . -type f -exec dos2unix {} \;

# Change to src directory
#WORKDIR /app/src


# Set the default command to run tests
CMD ["/bin/bash", "-c","rm -rf build && cmake -B build -S . &&  cmake --build build &&  ctest --test-dir build --output-on-failure"]