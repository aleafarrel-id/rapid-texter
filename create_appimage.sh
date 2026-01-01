#!/bin/bash

# Hentikan script jika ada error
set -e

echo "=== Memulai Pembuatan AppImage untuk Rapid Texter (Linux) ==="

# 1. Definisi Variabel
PROJECT_ROOT=$(pwd)
APP_DIR="$PROJECT_ROOT/AppDir"
BUILD_DIR="$PROJECT_ROOT/build_linux"

# Set source icon ke file .png
ICON_SOURCE="resources/app_icon.png" 

# 2. Cek appimagetool
APPIMAGETOOL="appimagetool-x86_64.AppImage"

if [ ! -f "$APPIMAGETOOL" ]; then
    echo "Downloading appimagetool..."
    wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x "$APPIMAGETOOL"
fi

# 3. Build Project dengan CMake
echo "Building project..."
if [ -d "$BUILD_DIR" ]; then rm -rf "$BUILD_DIR"; fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd "$PROJECT_ROOT"

# 4. Siapkan Struktur AppDir
echo "Creating AppDir structure..."
if [ -d "$APP_DIR" ]; then rm -rf "$APP_DIR"; fi
mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/lib"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APP_DIR/usr/share/applications"

# 5. Copy Executable & Assets
echo "Copying executable and assets..."

# Cari executable (bisa bernama RapidTexter atau rapid-texter)
if [ -f "$BUILD_DIR/RapidTexter" ]; then
    cp "$BUILD_DIR/RapidTexter" "$APP_DIR/usr/bin/rapid-texter"
elif [ -f "$BUILD_DIR/rapid-texter" ]; then
    cp "$BUILD_DIR/rapid-texter" "$APP_DIR/usr/bin/rapid-texter"
else
    echo "ERROR: Executable binary not found!"
    exit 1
fi

cp -r "assets" "$APP_DIR/usr/bin/"
cp -r "roll" "$APP_DIR/usr/bin/"

# 6. BUNDLE DEPENDENCIES MANUALLY
echo "Bundling dependencies manually..."

# Fungsi untuk copy library dan dependencies-nya secara rekursif
bundle_library() {
    local lib="$1"
    local libname=$(basename "$lib")
    
    # Skip jika sudah ada atau blacklisted
    if [ -f "$APP_DIR/usr/lib/$libname" ]; then
        return
    fi
    
    # Blacklist glibc core dan system libraries
    case "$libname" in
        libc.so*|libm.so*|libpthread.so*|libdl.so*|librt.so*|ld-linux*.so*|libgcc_s.so*|libstdc++.so*)
            return
            ;;
        # Blacklist libraries yang sangat terikat dengan sistem
        libnss*.so*|libresolv.so*|libBrokenLocale.so*)
            return
            ;;
    esac
    
    # Copy library
    if [ -f "$lib" ]; then
        echo "Bundling: $lib"
        cp -L "$lib" "$APP_DIR/usr/lib/" 2>/dev/null || true
        
        # Rekursif: bundle dependencies dari library ini juga
        for dep in $(ldd "$lib" 2>/dev/null | grep "=> /" | awk '{print $3}'); do
            bundle_library "$dep"
        done
    fi
}

# Bundle dependencies dari executable
EXECUTABLE="$APP_DIR/usr/bin/rapid-texter"
echo "Scanning dependencies from executable..."
for lib in $(ldd "$EXECUTABLE" | grep "=> /" | awk '{print $3}'); do
    bundle_library "$lib"
done

# Bundle dependencies dari library yang sudah di-copy (untuk memastikan lengkap)
echo "Scanning dependencies from bundled libraries..."
for bundled_lib in "$APP_DIR/usr/lib/"*.so*; do
    if [ -f "$bundled_lib" ]; then
        for lib in $(ldd "$bundled_lib" 2>/dev/null | grep "=> /" | awk '{print $3}'); do
            bundle_library "$lib"
        done
    fi
done

echo "Total libraries bundled: $(ls -1 "$APP_DIR/usr/lib/" | wc -l)"


# 7. SETUP ICON
echo "Setting up icon from $ICON_SOURCE..."

if [ -f "$ICON_SOURCE" ]; then
    echo "Found icon: $ICON_SOURCE"
    cp "$ICON_SOURCE" "$APP_DIR/app_icon.png"
    cp "$ICON_SOURCE" "$APP_DIR/.DirIcon"
    cp "$ICON_SOURCE" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/app_icon.png"
else
    echo "WARNING: $ICON_SOURCE tidak ditemukan!"
    echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==" | base64 -d > "$APP_DIR/app_icon.png"
    cp "$APP_DIR/app_icon.png" "$APP_DIR/.DirIcon"
fi

# 8. Buat file .desktop
echo "Creating .desktop file..."
cat > "$APP_DIR/rapid-texter.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Rapid Texter
Comment=Terminal-based typing speed game
Exec=rapid-texter
Icon=app_icon
Categories=Game;
Terminal=true
EOF

# Copy juga ke usr/share/applications
cp "$APP_DIR/rapid-texter.desktop" "$APP_DIR/usr/share/applications/"

# 9. Buat AppRun
echo "Creating AppRun script..."
cat > "$APP_DIR/AppRun" <<'APPRUN_EOF'
#!/bin/bash

# Get the directory where this AppRun script is located
APPDIR="$(dirname "$(readlink -f "$0")")"

# Setup library path
export LD_LIBRARY_PATH="$APPDIR/usr/lib:$LD_LIBRARY_PATH"

# Setup XDG paths
export XDG_DATA_DIRS="$APPDIR/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

# Function to run the game
run_game() {
    cd "$APPDIR/usr/bin"
    exec "$APPDIR/usr/bin/rapid-texter" "$@"
}

# Check if running in terminal
if [ -t 0 ] && [ -t 1 ]; then
    run_game "$@"
else
    # Launch in terminal emulator
    TARGET="${APPIMAGE:-$0}"
    
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal -- "$TARGET" "$@"
    elif command -v konsole &> /dev/null; then
        konsole -e "$TARGET" "$@"
    elif command -v xfce4-terminal &> /dev/null; then
        xfce4-terminal -x "$TARGET" "$@"
    elif command -v mate-terminal &> /dev/null; then
        mate-terminal -e "$TARGET"
    elif command -v lxterminal &> /dev/null; then
        lxterminal -e "$TARGET"
    elif command -v terminator &> /dev/null; then
        terminator -e "$TARGET"
    elif command -v alacritty &> /dev/null; then
        alacritty -e "$TARGET" "$@"
    elif command -v kitty &> /dev/null; then
        kitty "$TARGET" "$@"
    elif command -v xterm &> /dev/null; then
        xterm -e "$TARGET" "$@"
    else
        run_game "$@"
    fi
fi
APPRUN_EOF
chmod +x "$APP_DIR/AppRun"

# 10. Verify structure
echo "Verifying AppDir structure..."
echo "AppRun: $(file "$APP_DIR/AppRun")"
echo "Executable: $(file "$APP_DIR/usr/bin/rapid-texter")"
ls -la "$APP_DIR/usr/lib/"

# 11. Generate AppImage
echo "Packaging AppImage..."
export ARCH=x86_64

# Gunakan --appimage-extract-and-run untuk menghindari FUSE error
./$APPIMAGETOOL --appimage-extract-and-run "$APP_DIR" "RapidTexter-x86_64.AppImage"

echo "=== SELESAI! ==="
echo "File berhasil dibuat: RapidTexter-x86_64.AppImage"
echo "Sekarang file ini bisa di-double click dan akan otomatis membuka terminal."