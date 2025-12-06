#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-release"
APP_NAME="DesktopWidgetForLaundry"
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"
INSTALL_DIR="/Applications"

echo "Building $APP_NAME..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with Release build type
cmake -DCMAKE_BUILD_TYPE=Release "$SCRIPT_DIR"

# Build
cmake --build . --parallel

# Check if app was built
if [ ! -d "$APP_BUNDLE" ]; then
    echo "Error: Failed to build $APP_NAME.app"
    exit 1
fi

# Deploy Qt frameworks into the app bundle
echo "Deploying Qt frameworks..."
if command -v macdeployqt &> /dev/null; then
    macdeployqt "$APP_BUNDLE"
else
    echo "Warning: macdeployqt not found. The app may not work without Qt installed."
    echo "Install it with: brew install qt@5"
fi

# Remove old installation if exists
if [ -d "$INSTALL_DIR/$APP_NAME.app" ]; then
    echo "Removing old installation..."
    rm -rf "$INSTALL_DIR/$APP_NAME.app"
fi

# Copy to Applications
echo "Installing to $INSTALL_DIR..."
cp -R "$APP_BUNDLE" "$INSTALL_DIR/"

echo ""
echo "Installation complete!"
echo "You can find $APP_NAME in your Applications folder."
echo ""
echo "To uninstall, run: rm -rf \"$INSTALL_DIR/$APP_NAME.app\""
