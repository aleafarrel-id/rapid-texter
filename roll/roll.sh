#!/bin/bash

# Mendapatkan direktori tempat script ini berada
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Definisi file lokal
AUDIO_FILE="$DIR/roll.wav"
ART_FILE="$DIR/astley.txt"

# Bersihkan layar terminal
clear

# Simpan setting terminal asli (agar bisa dikembalikan nanti)
OLD_STTY=$(stty -g)

# Ubah Judul Terminal
printf "\033]0;Press ANY KEY to Stop\007"

# Fungsi Cleanup: Matikan audio, kembalikan terminal normal & judul jendela saat keluar
cleanup() {
    # Kembalikan setting terminal (agar ketikan user muncul lagi setelah script selesai)
    stty "$OLD_STTY"
    
    # Reset Judul Terminal ke default (misal: "Terminal")
    printf "\033]0;Terminal\007"
    
    # Kill background jobs (audio player)
    kill $(jobs -p) 2>/dev/null
    
    # Keluar dari script
    exit
}
# Jalankan cleanup jika user tekan Ctrl+C atau script dimatikan paksa
trap cleanup SIGINT SIGTERM

# Matikan echo terminal (agar tombol yang dipencet user tidak muncul/mengotori gambar)
stty -echo

# Mainkan Audio di Background
if [ -f "$AUDIO_FILE" ]; then
    if command -v paplay &> /dev/null; then
        paplay "$AUDIO_FILE" &
    elif command -v aplay &> /dev/null; then
        aplay -q "$AUDIO_FILE" &
    fi
else
    echo "Error: Audio ($AUDIO_FILE) tidak ditemukan!"
fi

# Tampilkan Gambar dengan Interupsi Keyboard
if [ -f "$ART_FILE" ]; then
    CNT=0
    
    # TEKNIK KHUSUS: Membaca file lewat "File Descriptor 3"
    # Kita pisahkan jalur baca file (fd 3) dan jalur baca keyboard (fd 0 / stdin)
    # supaya tidak bentrok.
    exec 3< "$ART_FILE"
    
    while IFS= read -r line <&3; do
        printf "%s\n" "$line"
        
        ((CNT++))
        if (( CNT % 5 == 0 )); then
            # PENGGANTI SLEEP: READ DENGAN TIMEOUT
            # read -t 0.005  : Tunggu 0.005 detik (Delay animasi kalibrasi)
            # -n 1           : Baca 1 karakter input
            # < /dev/tty     : Baca langsung dari keyboard user (bukan dari file gambar)
            
            if read -t 0.005 -n 1 < /dev/tty 2>/dev/null; then
                # Jika 'read' sukses (artinya user menekan tombol sebelum waktu habis),
                # Maka break loop (Hentikan Rick Roll).
                break
            fi
            
            # Jika user DIAM saja, 'read' akan timeout setelah 0.005 detik.
        fi
    done
    
    # Tutup File Descriptor 3
    exec 3<&-
else
    echo "Error: Gambar ($ART_FILE) tidak ditemukan!"
fi

# Selesai (atau di-stop user), panggil cleanup untuk matikan musik & reset judul
cleanup