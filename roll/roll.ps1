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
$originalTitle = $host.UI.RawUI.WindowTitle
$host.UI.RawUI.WindowTitle = "Press ANY KEY to Stop"

try {
    $bufferSize = $host.UI.RawUI.BufferSize
    $windowSize = $host.UI.RawUI.WindowSize
    
    if ($windowSize.Height -lt 34) {
        $windowSize.Height = 34
        $host.UI.RawUI.WindowSize = $windowSize
    }
    
    $bufferSize.Height = $windowSize.Height
    $bufferSize.Width = 85
    $host.UI.RawUI.BufferSize = $bufferSize
    
    [Console]::CursorVisible = $false
} catch {}

# --- FUNGSI UTAMA ---
function Start-RickRoll {
    # Load Video ke RAM
    $frames = [System.IO.File]::ReadAllLines($videoPath)
    
    Clear-Host
    Write-Host "Press ANY KEY to skip..." -ForegroundColor DarkGray
    
    # Setup Audio Player
    $player = New-Object System.Media.SoundPlayer $audioPath
    
    try {
        $player.Play()

        # ANSI Escape Codes untuk Windows
        $ESC = [char]27
        $cursorHome = "$ESC[2;1H"      # Move ke baris 2, kolom 1
        $clearScreen = "$ESC[0J"       # Clear dari cursor sampai akhir screen
        
        # Pre-build clear sequence untuk 32 baris dengan ANSI
        $clearLines = New-Object System.Text.StringBuilder 1500
        for ($i = 2; $i -le 33; $i++) {
            [void]$clearLines.Append("$ESC[$i;1H$ESC[2K")  # Move + clear entire line
        }
        $clearLinesStr = $clearLines.ToString()
        
        # Frame builder
        $sb = New-Object System.Text.StringBuilder 6000
        $lineCounter = 0
        
        $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
        $targetFrameTime = 33

        # Loop Animasi
        foreach ($line in $frames) {
            
            if ([Console]::KeyAvailable) {
                $null = [Console]::ReadKey($true)
                break 
            }

            # Pad line ke 80 karakter untuk overwrite
            $paddedLine = $line.PadRight(80)
            
            if ($lineCounter -eq 31) {
                [void]$sb.Append($paddedLine)
            } else {
                [void]$sb.Append($paddedLine + "`n")
            }
            $lineCounter++

            # Setiap 32 baris = 1 Frame
            if ($lineCounter -ge 32) {
                
                # --- SYNC FRAMERATE ---
                while ($stopwatch.ElapsedMilliseconds -lt $targetFrameTime) {
                    if ($targetFrameTime - $stopwatch.ElapsedMilliseconds -gt 5) {
                        Start-Sleep -Milliseconds 1
                    }
                }
                $stopwatch.Restart()

                # STRATEGI RENDERING OPTIMAL UNTUK WINDOWS:
                # 1. Clear semua line dengan ANSI (cepat)
                # 2. Home position
                # 3. Tulis frame baru
                # Semua dalam satu atomic write operation
                
                $finalOutput = $clearLinesStr + $cursorHome + $sb.ToString()
                
                # Single atomic write - paling cepat!
                [Console]::Write($finalOutput)
                
                # Reset
                [void]$sb.Clear()
                $lineCounter = 0
            }
        }
    } finally {
        # CLEANUP
        if ($player) {
            $player.Stop()
            $player.Dispose()
        }
        
        if ($originalTitle) {
            $host.UI.RawUI.WindowTitle = $originalTitle
        }
        
        try { [Console]::CursorVisible = $true } catch {}
        Clear-Host
    }
}

Start-RickRoll