# Rick Astley in your Terminal - Windows Edition (Fixed & Optimized)
# Original concept by Serene and Justine Tunney <3
# PowerShell Port Optimized for Windows 10/11
# Press Ctrl+C to exit

$ErrorActionPreference = 'SilentlyContinue'

# --- Configuration ---
$rickUrl = 'https://keroserene.net/lol'
$videoUrl = "$rickUrl/astley80.full.bz2"
$audioUrl = "$rickUrl/roll.s16"
$fps = 25

# --- ANSI Colors ---
$ESC = [char]27
$red = "$ESC[38;5;9m"
$yellow = "$ESC[38;5;216m"
$green = "$ESC[38;5;10m"
$purple = "$ESC[38;5;171m"
$reset = "$ESC[0m"
$hideCursor = "$ESC[?25l"
$showCursor = "$ESC[?25h"

# --- Setup Console ---
$HOST.UI.RawUI.WindowTitle = "Never Gonna Give You Up"
Write-Host "${green}Rick Astley performs ♪ Never Gonna Give You Up ♪ on your terminal.${reset}"
Write-Host "${purple}Loading assets... (please wait)${reset}"

# Hide cursor properly using ANSI if compatible, fallback to .NET
try { [Console]::CursorVisible = $false } catch {}
Write-Host -NoNewline $hideCursor

# --- Helper: Create WAV Header for Raw Audio ---
# The original audio is 8000Hz, 16-bit, Mono, PCM (Raw .s16)
function Add-WavHeader {
    param([byte[]]$RawData)
    
    $SampleRate = 8000
    $Channels = 1
    $BitsPerSample = 16
    
    $ByteRate = $SampleRate * $Channels * ($BitsPerSample / 8)
    $BlockAlign = $Channels * ($BitsPerSample / 8)
    $SubChunk2Size = $RawData.Length
    $ChunkSize = 36 + $SubChunk2Size

    $Header = New-Object byte[] 44
    
    # RIFF header
    [System.Buffer]::BlockCopy([System.Text.Encoding]::ASCII.GetBytes("RIFF"), 0, $Header, 0, 4)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([int]$ChunkSize), 0, $Header, 4, 4)
    [System.Buffer]::BlockCopy([System.Text.Encoding]::ASCII.GetBytes("WAVE"), 0, $Header, 8, 4)
    
    # fmt subchunk
    [System.Buffer]::BlockCopy([System.Text.Encoding]::ASCII.GetBytes("fmt "), 0, $Header, 12, 4)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([int]16), 0, $Header, 16, 4) # Subchunk1Size
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([short]1), 0, $Header, 20, 2) # AudioFormat (PCM)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([short]$Channels), 0, $Header, 22, 2)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([int]$SampleRate), 0, $Header, 24, 4)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([int]$ByteRate), 0, $Header, 28, 4)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([short]$BlockAlign), 0, $Header, 32, 2)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([short]$BitsPerSample), 0, $Header, 34, 2)
    
    # data subchunk
    [System.Buffer]::BlockCopy([System.Text.Encoding]::ASCII.GetBytes("data"), 0, $Header, 36, 4)
    [System.Buffer]::BlockCopy([BitConverter]::GetBytes([int]$SubChunk2Size), 0, $Header, 40, 4)

    return ,($Header + $RawData)
}

# --- Main Logic ---
try {
    $tempDir = [System.IO.Path]::GetTempPath()
    $bz2File = Join-Path $tempDir "roll.bz2"
    $rawAudioFile = Join-Path $tempDir "roll.s16"
    $wavFile = Join-Path $tempDir "roll.wav"
    $videoTxt = Join-Path $tempDir "roll.txt"

    # 1. Download & Process Audio
    # Write-Host "${yellow}Fetching audio...${reset}"
    if (-not (Test-Path $wavFile)) {
        try {
            Invoke-WebRequest -Uri $audioUrl -OutFile $rawAudioFile -UseBasicParsing
            $audioBytes = [System.IO.File]::ReadAllBytes($rawAudioFile)
            $wavBytes = Add-WavHeader -RawData $audioBytes
            [System.IO.File]::WriteAllBytes($wavFile, $wavBytes)
            Remove-Item $rawAudioFile -Force
        } catch {
            Write-Host "${red}Audio download failed. Continuing silent.${reset}"
        }
    }

    # 2. Start Audio Player (Background)
    $player = $null
    if (Test-Path $wavFile) {
        $player = New-Object System.Media.SoundPlayer
        $player.SoundLocation = $wavFile
        $player.Load()
        $player.Play()
    }

    # 3. Download & Decompress Video
    # Write-Host "${yellow}Fetching video...${reset}"
    
    if (-not (Test-Path $videoTxt)) {
        Invoke-WebRequest -Uri $videoUrl -OutFile $bz2File -UseBasicParsing
        
        # Check for 'tar' (Available on Win10/11) to decompress bz2
        if (Get-Command "tar" -ErrorAction SilentlyContinue) {
            # tar -xf input.bz2 -O > output.txt (Extract to stdout)
            # PowerShell piping is tricky with binary, so we let tar write to stdout and capture text
            $proc = Start-Process -FilePath "tar" -ArgumentList "-xOf", "$bz2File" -NoNewWindow -PassThru -RedirectStandardOutput $videoTxt
            $proc.WaitForExit()
        } else {
             Write-Host "${red}Error: 'tar' command not found. Ensure you are on Windows 10/11.${reset}"
             exit
        }
        Remove-Item $bz2File -Force
    }

    # 4. Play Video
    $content = [System.IO.File]::ReadAllLines($videoTxt)
    $frameCount = $content.Length / 32 # Each frame is 32 lines high
    $delayTicks = [long](10000000 / $fps) # Ticks per frame
    
    Clear-Host
    
    # Pre-calculate screen buffer position to avoid flickering (Don't use Clear-Host in loop)
    $origin = New-Object System.Management.Automation.Host.Coordinates(0, 0)
    
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $lastFrame = -1

    # Animation Loop
    for ($i = 0; $i -lt $frameCount; $i++) {
        # Timing Logic
        $targetTime = $i * (1.0 / $fps)
        $elapsed = $stopwatch.Elapsed.TotalSeconds
        
        if ($elapsed -lt $targetTime) {
            $sleepTime = ($targetTime - $elapsed) * 1000
            if ($sleepTime -gt 0) { Start-Sleep -Milliseconds $sleepTime }
        }

        # Rendering
        # Only draw if we haven't skipped too far ahead
        # Using [Console]::Out.Write is faster than Write-Host
        [Console]::SetCursorPosition(0, 0)
        
        $startLine = $i * 32
        $frameLines = $content[$startLine..($startLine + 31)]
        
        # Join lines for a single write operation to reduce tearing
        $frameText = $frameLines -join "`n"
        [Console]::Out.Write($frameText)

        # Handle Exit Key
        if ([Console]::KeyAvailable) {
            $key = [Console]::ReadKey($true)
            if ($key.Key -eq 'Escape' -or ($key.Modifiers -eq 'Control' -and $key.Key -eq 'C')) {
                break
            }
        }
    }

} catch {
    Write-Host "${red}Error: $_${reset}"
} finally {
    # Cleanup
    if ($player) { $player.Stop(); $player.Dispose() }
    
    # Restore cursor
    Write-Host -NoNewline $showCursor
    try { [Console]::CursorVisible = $true } catch {}
    
    # Optional cleanup of cached files
    # Remove-Item $wavFile -ErrorAction SilentlyContinue
    # Remove-Item $videoTxt -ErrorAction SilentlyContinue
    
    Write-Host "`n${purple}<3${reset}"
}