# PowerShell script to download Windows libretro cores
# Run this from the project root directory

$coresDir = "cores"
$baseUrl = "https://buildbot.libretro.com/nightly/windows/x86_64/latest"

# Create cores directory if it doesn't exist
if (!(Test-Path $coresDir)) {
    New-Item -ItemType Directory -Path $coresDir
    Write-Host "Created cores directory"
}

# List of popular cores to download
$cores = @(
    "mgba_libretro.dll.zip",              # Game Boy Advance
    "gambatte_libretro.dll.zip",          # Game Boy / Game Boy Color
    "snes9x_libretro.dll.zip",            # SNES
    "fceumm_libretro.dll.zip",            # NES
    "genesis_plus_gx_libretro.dll.zip",   # Genesis/Mega Drive/SMS/Game Gear
    "beetle_psx_hw_libretro.dll.zip",     # PlayStation
    "mupen64plus_next_libretro.dll.zip",  # Nintendo 64
    "ppsspp_libretro.dll.zip",            # PSP
    "fbneo_libretro.dll.zip",             # Arcade (FinalBurn Neo)
    "desmume_libretro.dll.zip"            # Nintendo DS
)

Write-Host "`nDownloading libretro cores for Windows...`n" -ForegroundColor Green

foreach ($core in $cores) {
    $coreName = $core -replace "\.zip$", ""
    $url = "$baseUrl/$core"
    $zipPath = "$coresDir\$core"
    $dllPath = "$coresDir\$coreName"
    
    # Skip if already downloaded
    if (Test-Path $dllPath) {
        Write-Host "[SKIP] $coreName already exists" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "Downloading $coreName..." -ForegroundColor Cyan
    
    try {
        # Download the zip file
        Invoke-WebRequest -Uri $url -OutFile $zipPath -ErrorAction Stop
        
        # Extract the DLL
        Expand-Archive -Path $zipPath -DestinationPath $coresDir -Force
        
        # Remove the zip file
        Remove-Item $zipPath
        
        Write-Host "[OK] $coreName downloaded successfully" -ForegroundColor Green
    }
    catch {
        Write-Host "[FAIL] Failed to download $coreName : $_" -ForegroundColor Red
    }
}

Write-Host "`nCore download complete!" -ForegroundColor Green
Write-Host "Cores are located in: $coresDir" -ForegroundColor Green
Write-Host "`nYou can now add ROMs to the ROMS directory and run pdEMU.exe" -ForegroundColor Yellow
