# PowerShell script to build and run pdEMU on Windows

Write-Host "===================================" -ForegroundColor Cyan
Write-Host "pdEMU Frontend - Build & Run Script" -ForegroundColor Cyan
Write-Host "===================================" -ForegroundColor Cyan
Write-Host ""

# Add MSYS2 to PATH if available
if (Test-Path "C:\msys64\mingw64\bin") {
    $env:Path = "C:\msys64\mingw64\bin;" + $env:Path
}

# Check if executable exists
if (Test-Path "build\pdEMU.exe") {
    Write-Host "Executable found: build\pdEMU.exe" -ForegroundColor Green
    Write-Host ""
    Write-Host "Launching pdEMU Frontend..." -ForegroundColor Cyan
    Write-Host ""
    Start-Process ".\build\pdEMU.exe"
} else {
    Write-Host "Executable not found." -ForegroundColor Yellow
    Write-Host "Please build the project first using MSYS2:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  cd build" -ForegroundColor White
    Write-Host "  cmake -G 'MinGW Makefiles' .." -ForegroundColor White
    Write-Host "  mingw32-make" -ForegroundColor White
    Write-Host ""
    exit 1
}
