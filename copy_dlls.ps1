# PowerShell script to copy required DLLs to build directory
# This allows pdEMU.exe to run standalone without MSYS2 in PATH

$MSYS2_BIN = "C:\msys64\mingw64\bin"
$BUILD_DIR = "build"

if (!(Test-Path $MSYS2_BIN)) {
    Write-Host "Error: MSYS2 not found at $MSYS2_BIN" -ForegroundColor Red
    exit 1
}

if (!(Test-Path $BUILD_DIR)) {
    Write-Host "Error: Build directory not found. Please build the project first." -ForegroundColor Red
    exit 1
}

Write-Host "Copying required DLLs to build directory..." -ForegroundColor Cyan
Write-Host ""

# List of required DLLs
$dlls = @(
    "SDL2.dll",
    "SDL2_image.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    "libpng16-16.dll",
    "zlib1.dll"
)

$copied = 0
$missing = 0

foreach ($dll in $dlls) {
    $source = Join-Path $MSYS2_BIN $dll
    $dest = Join-Path $BUILD_DIR $dll
    
    if (Test-Path $source) {
        Copy-Item $source $dest -Force
        Write-Host "[OK] Copied $dll" -ForegroundColor Green
        $copied++
    } else {
        Write-Host "[SKIP] $dll not found" -ForegroundColor Yellow
        $missing++
    }
}

Write-Host ""
Write-Host "Copied $copied DLLs, $missing not found" -ForegroundColor Cyan
Write-Host ""

# Copy cores, ROMS, BIOS, and SAVES folders
Write-Host "Copying resource folders to build directory..." -ForegroundColor Cyan

$folders = @("cores", "ROMS", "BIOS", "SAVES")
foreach ($folder in $folders) {
    if (Test-Path $folder) {
        $dest = Join-Path $BUILD_DIR $folder
        if (!(Test-Path $dest)) {
            New-Item -ItemType Directory -Path $dest -Force | Out-Null
        }
        # Copy contents
        Copy-Item "$folder\*" $dest -Recurse -Force
        $itemCount = (Get-ChildItem $folder -File -Recurse).Count
        Write-Host "[OK] Copied $folder ($itemCount files)" -ForegroundColor Green
    } else {
        Write-Host "[SKIP] $folder directory not found" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "pdEMU.exe can now run standalone!" -ForegroundColor Green
Write-Host "You can run: .\build\pdEMU.exe" -ForegroundColor White
