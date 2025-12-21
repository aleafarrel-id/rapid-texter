<#
.SYNOPSIS
    Rick Astley Offline Player untuk Rapid Texter
    Terintegrasi dengan C++ Game Engine.
    Fitur Baru: Tekan tombol apa saja untuk berhenti.
#>

$ErrorActionPreference = "SilentlyContinue"
$currentDir = $PSScriptRoot

# --- SETUP ANSI ---
if ([System.Environment]::OSVersion.Version.Major -ge 10) {
    $code = @'
    [DllImport("kernel32.dll")] public static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);
    [DllImport("kernel32.dll")] public static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);
    [DllImport("kernel32.dll")] public static extern IntPtr GetStdHandle(int nStdHandle);
'@
    try {
        if (-not ([System.Management.Automation.PSTypeName]'Win32Rick').Type) {
            $null = Add-Type -MemberDefinition $code -Name "Win32Rick" -Namespace Win32 -PassThru
        }
        $type = [Win32.Win32Rick]
        $handle = $type::GetStdHandle(-11)
        $mode = 0
        [void]$type::GetConsoleMode($handle, [ref]$mode)
        [void]$type::SetConsoleMode($handle, $mode -bor 4) 
    } catch {}
}

# --- KONFIGURASI PATH ---
# Script ini berada di dalam folder 'roll/' bersama asetnya,
# Cukup cari file di direktori yang sama ($currentDir).
$audioPath = Join-Path $currentDir "roll.wav"
$videoPath = Join-Path $currentDir "astley.txt"

# --- ERROR HANDLING ---
if (-not (Test-Path $audioPath)) {
    Write-Host "Error: roll.wav tidak ditemukan di folder roll/." -ForegroundColor Red
    Start-Sleep -Seconds 2
    exit
}
if (-not (Test-Path $videoPath)) {
    Write-Host "Error: astley.txt tidak ditemukan di folder roll/." -ForegroundColor Red
    Start-Sleep -Seconds 2
    exit
}

# --- SETUP TAMPILAN ---
# Simpan judul asli
$originalTitle = $host.UI.RawUI.WindowTitle
$host.UI.RawUI.WindowTitle = "Press ANY KEY to Stop"
try { [Console]::CursorVisible = $false } catch {}

# --- FUNGSI UTAMA ---
function Start-RickRoll {
    # 1. Load Video ke RAM
    $frames = [System.IO.File]::ReadAllLines($videoPath)
    
    Clear-Host
    Write-Host "Press ANY KEY to skip..." -ForegroundColor DarkGray
    
    # 2. Setup Audio Player
    $player = New-Object System.Media.SoundPlayer $audioPath
    
    try {
        $player.Play() # Play Async

        $sb = New-Object System.Text.StringBuilder
        $lineCounter = 0
        [Console]::SetCursorPosition(0,0)

        # 3. Loop Animasi
        foreach ($line in $frames) {
            
            # --- FITUR STOP ---
            # Jika user menekan tombol apa saja, hentikan loop
            if ([Console]::KeyAvailable) {
                # Baca key agar tidak tertinggal di buffer input C++ nanti
                $null = [Console]::ReadKey($true)
                break 
            }

            [void]$sb.Append($line + "`n")
            $lineCounter++

            # Setiap 32 baris = 1 Frame
            if ($lineCounter -ge 32) {
                [Console]::SetCursorPosition(0,0)
                
                # Print frame
                [Console]::Out.Write($sb.ToString().TrimEnd())
                
                # Reset
                [void]$sb.Clear()
                $lineCounter = 0

                # Delay 35ms (~25-28 FPS)
                Start-Sleep -Milliseconds 35
            }
        }
    } finally {
        # 4. CLEANUP (Sangat Penting)
        # Memastikan audio mati total sebelum kembali ke C++
        if ($player) {
            $player.Stop()
            $player.Dispose()
        }
        
        # Restore judul asli
        if ($originalTitle) {
            $host.UI.RawUI.WindowTitle = $originalTitle
        }
        
        try { [Console]::CursorVisible = $true } catch {}
        Clear-Host
    }
}

Start-RickRoll