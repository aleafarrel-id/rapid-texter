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

# 2. Cek tool AppImage & LinuxDeploy
if [ ! -f "appimagetool-x86_64.AppImage" ]; then
    echo "Downloading appimagetool..."
    wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    echo "Downloading linuxdeploy..."
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

# 3. Build Project dengan CMake
echo "Building project..."
if [ -d "$BUILD_DIR" ]; then rm -rf "$BUILD_DIR"; fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make
cd "$PROJECT_ROOT"

# 4. Siapkan Struktur AppDir
echo "Creating AppDir structure..."
if [ -d "$APP_DIR" ]; then rm -rf "$APP_DIR"; fi
mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"

# 5. Copy Executable & Assets
echo "Copying executable and assets..."

# Cari executable (bisa bernama RapidTexter atau rapid-texter)
EXE_NAME="rapid-texter"
if [ -f "$BUILD_DIR/RapidTexter" ]; then
    cp "$BUILD_DIR/RapidTexter" "$APP_DIR/usr/bin/$EXE_NAME"
elif [ -f "$BUILD_DIR/rapid-texter" ]; then
    cp "$BUILD_DIR/rapid-texter" "$APP_DIR/usr/bin/$EXE_NAME"
else
    echo "ERROR: Executable binary not found!"
    exit 1
fi

cp -r "assets" "$APP_DIR/usr/bin/"
cp -r "roll" "$APP_DIR/usr/bin/"

# 6. SETUP ICON
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

# 7. Buat file .desktop
echo "Creating .desktop file..."
cat > "$APP_DIR/rapid-texter.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Rapid Texter
Comment=Terminal-based typing speed game
Exec=$EXE_NAME
Icon=app_icon
Categories=Game;
Terminal=true
EOF

# 8. BUNDLE DEPENDENCIES (New Step)
echo "Bundling dependencies with linuxdeploy..."
# Kita gunakan linuxdeploy hanya untuk copy library ke usr/lib
./linuxdeploy-x86_64.AppImage --appdir "$APP_DIR" --executable "$APP_DIR/usr/bin/$EXE_NAME" --desktop-file "$APP_DIR/rapid-texter.desktop" --icon-file "$ICON_SOURCE"

# 9. Buat AppRun (SMART LAUNCHER UPDATE)
# Script ini menimpa AppRun generik dari linuxdeploy agar bisa handle terminal logic
echo "Creating custom AppRun script..."
cat > "$APP_DIR/AppRun" <<\EOF
#!/bin/bash

# Fungsi untuk menjalankan game yang sebenarnya
run_game() {
    # Cari lokasi script ini berada
    HERE="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
    
    # SETUP LIBRARY PATH (PENTING AGAR LIBRARY TERBUNDLE TERBACA)
    export LD_LIBRARY_PATH="$HERE/usr/lib:$LD_LIBRARY_PATH"
    
    cd "${HERE}/usr/bin"
    ./rapid-texter "$@"
}

# LOGIC UTAMA: Cek apakah dijalankan di dalam terminal?
if [ -t 0 ]; then
    # YA, ini dijalankan dari terminal -> Langsung mainkan!
    run_game "$@"
else
    # TIDAK, ini dijalankan dari GUI (Double Click) -> Buka terminal dulu!
    
    # Ambil lokasi file AppImage ini sendiri
    TARGET="$APPIMAGE"
    if [ -z "$TARGET" ]; then TARGET="$0"; fi

    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal -- "$TARGET" "$@"
    elif command -v konsole &> /dev/null; then
        konsole -e "$TARGET" "$@"
    elif command -v xfce4-terminal &> /dev/null; then
        xfce4-terminal -e "$TARGET" "$@"
    elif command -v terminator &> /dev/null; then
        terminator -e "$TARGET" "$@"
    elif command -v xterm &> /dev/null; then
        xterm -e "$TARGET" "$@"
    else
        run_game "$@"
    fi
fi
EOF
chmod +x "$APP_DIR/AppRun"

# 9. Generate AppImage
echo "Packaging AppImage..."
export ARCH=x86_64
./appimagetool-x86_64.AppImage "$APP_DIR" "RapidTexter-x86_64.AppImage"

echo "=== SELESAI! ==="
echo "File berhasil dibuat: RapidTexter-x86_64.AppImage"
echo "Sekarang file ini bisa di-double click dan akan otomatis membuka terminal."