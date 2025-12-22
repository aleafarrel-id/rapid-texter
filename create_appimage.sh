#!/bin/bash

# Hentikan script jika ada error
set -e

echo "=== Memulai Pembuatan AppImage untuk Rapid Texter ==="

# Pastikan kita berada di root project
PROJECT_ROOT=$(pwd)
APP_DIR="$PROJECT_ROOT/AppDir"
BUILD_DIR="$PROJECT_ROOT/build_linux"

# Cek apakah appimagetool sudah ada, jika belum download
if [ ! -f "appimagetool-x86_64.AppImage" ]; then
    echo "Downloading appimagetool..."
    wget -q https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

# Build Project dengan CMake
echo "Building project..."
if [ -d "$BUILD_DIR" ]; then rm -rf "$BUILD_DIR"; fi
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make
cd "$PROJECT_ROOT"

# Siapkan Struktur AppDir
echo "Creating AppDir structure..."
if [ -d "$APP_DIR" ]; then rm -rf "$APP_DIR"; fi
mkdir -p "$APP_DIR/usr/bin"
mkdir -p "$APP_DIR/usr/share/icons"

# Copy Executable dan Assets
echo "Copying executable and assets..."

# Deteksi nama file executable (RapidTexter atau rapid-texter)
# Akan copy dan rename menjadi 'rapid-texter' (huruf kecil) agar konsisten di dalam AppImage
if [ -f "$BUILD_DIR/RapidTexter" ]; then
    echo "Found executable: RapidTexter"
    cp "$BUILD_DIR/RapidTexter" "$APP_DIR/usr/bin/rapid-texter"
elif [ -f "$BUILD_DIR/rapid-texter" ]; then
    echo "Found executable: rapid-texter"
    cp "$BUILD_DIR/rapid-texter" "$APP_DIR/usr/bin/rapid-texter"
else
    echo "ERROR: Executable binary not found in $BUILD_DIR!"
    echo "Isi folder build:"
    ls -l "$BUILD_DIR"
    exit 1
fi

# Copy assets
cp -r "assets" "$APP_DIR/usr/bin/"
cp -r "roll" "$APP_DIR/usr/bin/"

# Buat file .desktop (Metadata untuk menu Linux)
# Icon diset ke 'rapid-texter' agar cocok dengan file png yang akan kita buat
# Categories disederhanakan agar tidak warning
echo "Creating .desktop file..."
cat > "$APP_DIR/rapid-texter.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Rapid Texter
Comment=A simple typing game
Exec=rapid-texter
Icon=rapid-texter
Categories=Game;
Terminal=true
EOF

# Buat file AppRun (Script peluncur utama)
echo "Creating AppRun script..."
cat > "$APP_DIR/AppRun" <<EOF
#!/bin/bash
# Pindah ke direktori tempat binary dan assets berada
cd "\$(dirname "\$0")/usr/bin"
# Jalankan aplikasi (nama file sudah distandarisasi jadi rapid-texter di langkah 4)
./rapid-texter "\$@"
EOF

chmod +x "$APP_DIR/AppRun"

# Siapkan Icon
# Membuat file PNG valid (1x1 pixel merah) agar appimagetool tidak error karena file kosong/tidak ditemukan
echo "Creating icon file..."
echo "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==" | base64 -d > "$APP_DIR/rapid-texter.png"
# Juga copy ke folder icons standar agar lebih rapi (opsional tapi bagus)
cp "$APP_DIR/rapid-texter.png" "$APP_DIR/.DirIcon"

# 8. Generate AppImage
echo "Packaging AppImage..."
export ARCH=x86_64
./appimagetool-x86_64.AppImage "$APP_DIR" "RapidTexter-x86_64.AppImage"

echo "=== Selesai! ==="
echo "File portable kamu ada di: RapidTexter-x86_64.AppImage"
echo "Kamu bisa menjalankannya dengan: ./RapidTexter-x86_64.AppImage"