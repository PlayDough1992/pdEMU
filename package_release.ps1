# pdEMU Release Packaging Script
# Packages all required files for distribution

$ErrorActionPreference = "Stop"

# Paths
$buildDir = "build"
$releaseDir = "pdEMU_v1.0_Beta_Windows"

# Clean up old release folder if it exists
if (Test-Path $releaseDir) {
    Write-Host "Removing old release folder..."
    Remove-Item $releaseDir -Recurse -Force
}

# Create release folder structure
Write-Host "Creating release folder structure..."
New-Item -ItemType Directory -Path $releaseDir | Out-Null
New-Item -ItemType Directory -Path "$releaseDir\cores" | Out-Null
New-Item -ItemType Directory -Path "$releaseDir\ROMS" | Out-Null
New-Item -ItemType Directory -Path "$releaseDir\BIOS" | Out-Null
New-Item -ItemType Directory -Path "$releaseDir\SAVES" | Out-Null
New-Item -ItemType Directory -Path "$releaseDir\controller_profiles" | Out-Null

# Copy executable
Write-Host "Copying executable..."
Copy-Item "$buildDir\pdEMU.exe" -Destination $releaseDir

# Copy all required DLLs from REQUIRED_DLLs folder
Write-Host "Copying required DLL files..."
if (Test-Path "REQUIRED_DLLs") {
    Get-ChildItem -Path "REQUIRED_DLLs" -Filter "*.dll" | ForEach-Object {
        Copy-Item $_.FullName -Destination $releaseDir -Force
        Write-Host "  Copied: $($_.Name)"
    }
} else {
    Write-Host "  Warning: REQUIRED_DLLs folder not found, falling back to build directory"
    Get-ChildItem -Path $buildDir -Filter "*.dll" -Recurse | ForEach-Object {
        Copy-Item $_.FullName -Destination $releaseDir -Force
        Write-Host "  Copied: $($_.Name)"
    }
}

# Copy cores
Write-Host "Copying cores..."
if (Test-Path "$buildDir\cores") {
    Get-ChildItem -Path "$buildDir\cores" -Filter "*.dll" | ForEach-Object {
        Copy-Item $_.FullName -Destination "$releaseDir\cores" -Force
        Write-Host "  Copied: $($_.Name)"
    }
} else {
    Write-Host "  Warning: cores folder not found in build directory"
}

# Copy config.json
Write-Host "Copying config.json..."
Copy-Item "config.json" -Destination $releaseDir

# Copy controller profiles if they exist
Write-Host "Copying controller profiles..."
if (Test-Path "controller_profiles\ui_profile.json") {
    Copy-Item "controller_profiles\ui_profile.json" -Destination "$releaseDir\controller_profiles"
}

# Copy release notes
Write-Host "Copying release notes..."
if (Test-Path "RELEASE_NOTES_v1.0.md") {
    Copy-Item "RELEASE_NOTES_v1.0.md" -Destination $releaseDir
}

# Create README for empty folders
Write-Host "Creating placeholder files..."
@"
Place your ROM files here.
pdEMU will automatically detect and organize them by system.
"@ | Out-File -FilePath "$releaseDir\ROMS\README.txt" -Encoding UTF8

@"
Place BIOS files here if required by certain cores.
Common files needed:
- PlayStation 1: scph5500.bin, scph5501.bin, scph5502.bin
- Other systems may require specific BIOS files
"@ | Out-File -FilePath "$releaseDir\BIOS\README.txt" -Encoding UTF8

@"
Save states and save files will be stored here automatically.
"@ | Out-File -FilePath "$releaseDir\SAVES\README.txt" -Encoding UTF8

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Release package created successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Release folder: $releaseDir"
Write-Host ""
Write-Host "Contents:"
Get-ChildItem -Path $releaseDir -Recurse | Select-Object FullName, Length | Format-Table -AutoSize

Write-Host ""
Write-Host "To create a ZIP archive, run:"
Write-Host "Compress-Archive -Path '$releaseDir' -DestinationPath 'pdEMU_v1.0_Beta_Windows.zip'" -ForegroundColor Yellow
