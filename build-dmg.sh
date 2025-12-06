#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-release"
APP_NAME="DesktopWidgetForLaundry"
APP_BUNDLE="$BUILD_DIR/$APP_NAME.app"
DMG_NAME="LaundryTimer"
DMG_PATH="$SCRIPT_DIR/$DMG_NAME.dmg"
STAGING_DIR="$BUILD_DIR/dmg-staging"

echo "=== Building $APP_NAME ==="

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with Release build type
cmake -DCMAKE_BUILD_TYPE=Release "$SCRIPT_DIR"

# Build
cmake --build . --parallel

if [ ! -d "$APP_BUNDLE" ]; then
    echo "Error: Failed to build $APP_NAME.app"
    exit 1
fi

echo "=== Deploying Qt frameworks ==="
if command -v macdeployqt &> /dev/null; then
    macdeployqt "$APP_BUNDLE"
else
    # Try to find macdeployqt from Homebrew Qt
    QT_DEPLOY=$(find /opt/homebrew/opt/qt@5 /usr/local/opt/qt@5 -name "macdeployqt" 2>/dev/null | head -1)
    if [ -n "$QT_DEPLOY" ]; then
        "$QT_DEPLOY" "$APP_BUNDLE"
    else
        echo "Warning: macdeployqt not found. App may not work on systems without Qt."
        echo "Install with: brew install qt@5"
    fi
fi

echo "=== Signing app ==="
codesign --force --deep --sign - "$APP_BUNDLE"

echo "=== Creating DMG ==="

# Clean up previous staging/dmg
rm -rf "$STAGING_DIR"
rm -f "$DMG_PATH"

# Create staging directory
mkdir -p "$STAGING_DIR"

# Copy app to staging
cp -R "$APP_BUNDLE" "$STAGING_DIR/"

# Rename to friendly name
mv "$STAGING_DIR/$APP_NAME.app" "$STAGING_DIR/Laundry Timer.app"

# Create symlink to Applications
ln -s /Applications "$STAGING_DIR/Applications"

# Create the DMG
hdiutil create -volname "$DMG_NAME" \
    -srcfolder "$STAGING_DIR" \
    -ov -format UDZO \
    "$DMG_PATH"

# Clean up staging
rm -rf "$STAGING_DIR"

echo ""
echo "=== Done! ==="
echo "DMG created: $DMG_PATH"
echo ""
echo "To distribute:"
echo "  1. Share the $DMG_NAME.dmg file"
echo "  2. Users open it and drag 'Laundry Timer' to Applications"
