# PowerShell script to build and run pdEMU on Windows

Write-Host "===================================" -ForegroundColor Cyan
Write-Host "pdEMU Frontend - Build & Run Script" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (!(Test-Path "build")) {
    Write-Host "Build directory not found. Creating..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Check if already built
if (Test-Path "build\pdEMU.exe") {
    Write-Host "✓ Executable found: build\pdEMU.exe" -ForegroundColor Green
    Write-Host ""
    Write-Host "Launching pdEMU Frontend..." -ForegroundColor Cyan
    Write-Host ""
    
    # Run from MSYS2 if available, otherwise try directly
    if (Test-Path "C:\msys64\mingw64\bin") {
        $env:Path = "C:\msys64\mingw64\bin;" + $env:Path
    }
    
    & ".\build\pdEMU.exe"
} else {
    Write-Host "Executable not found. Building..." -ForegroundColor Yellow
    Write-Host ""
    
    # Check if we're in MSYS2 environment
    $inMSYS2 = $null -ne $env:MSYSTEM
    
    if (!$inMSYS2) {
        Write-Host "Please run this from MSYS2 MinGW64 terminal to build, or build manually:" -ForegroundColor Red
        Write-Host ""
        Write-Host "  cd build" -ForegroundColor Yellow
        Write-Host "  cmake -G `"MinGW Makefiles`" .." -ForegroundColor Yellow
        Write-Host "  mingw32-make" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Or run from MSYS2 terminal:" -ForegroundColor Yellow
        Write-Host "  ./run.sh" -ForegroundColor Yellow
        exit 1
    }
    
    # Build using mingw32-make
    Set-Location build
    cmake -G "MinGW Makefiles" ..
    mingw32-make
    Set-Location ..
    
    Write-Host ""
    if (Test-Path "build\pdEMU.exe") {
        Write-Host "✓ Build successful!" -ForegroundColor Green
        Write-Host ""
        Write-Host "Launching pdEMU Frontend..." -ForegroundColor Cyan
        Write-Host ""
        & ".\build\pdEMU.exe"
    } else {
        Write-Host "✗ Build failed. Please check the errors above." -ForegroundColor Red
        exit 1
    }
}
