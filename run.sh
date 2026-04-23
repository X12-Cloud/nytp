#!/bin/bash

# Configuration
BIN_NAME="nytp"
BUILD_DIR="build"

# Check for -p flag
INSTALL_TO_PATH=false
for arg in "$@"; do
    if [ "$arg" == "-p" ] || [ "$arg" == "--path" ]; then
        INSTALL_TO_PATH=true
    fi
done

echo "--- Starting Build ---"
# rm -rf $BUILD_DIR
# cmake -B $BUILD_DIR
cmake --build $BUILD_DIR

if [ $? -eq 0 ]; then
    echo "Build successful."

    if [ "$INSTALL_TO_PATH" = true ]; then
        echo "Installing $BIN_NAME to /usr/bin/..."
        if [ -f "$BUILD_DIR/$BIN_NAME" ]; then
            sudo cp "$BUILD_DIR/$BIN_NAME" "/usr/bin/$BIN_NAME"
            sudo chmod +x "/usr/bin/$BIN_NAME"
            echo "You can now run '$BIN_NAME' from anywhere."
        else
            echo "Error: Binary $BUILD_DIR/$BIN_NAME not found."
        fi
    fi
else
    echo "Build failed."
    exit 1
fi
