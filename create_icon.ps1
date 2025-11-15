Add-Type -AssemblyName System.Drawing

# Load the logo PNG
$logoPath = "D:\Projects_other\pdEMU_Windows\pdEMU_Icon.png"
$outputPath = "D:\Projects_other\pdEMU_Windows\pdemu2.ico"

# Load original image
$original = [System.Drawing.Image]::FromFile($logoPath)

# Create 256x256 bitmap with dark background
$size = 256
$ratio = $original.Width / $original.Height
if ($ratio -gt 1) {
    $width = $size
    $height = [int]($size / $ratio)
} else {
    $height = $size
    $width = [int]($size * $ratio)
}

# Create dark background bitmap
$bitmap = New-Object System.Drawing.Bitmap($size, $size)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)

# Set dark background (matching UI theme)
$darkBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(30, 30, 35))
$graphics.FillRectangle($darkBrush, 0, 0, $size, $size)

# Set high quality rendering
$graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
$graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality

# Center the logo
$x = ($size - $width) / 2
$y = ($size - $height) / 2

# Draw the logo
$graphics.DrawImage($original, $x, $y, $width, $height)

# Save to temporary PNG first
$tempPng = [System.IO.Path]::GetTempFileName() + ".png"
$bitmap.Save($tempPng, [System.Drawing.Imaging.ImageFormat]::Png)

$graphics.Dispose()
$bitmap.Dispose()
$darkBrush.Dispose()
$original.Dispose()

# Load as icon and save
$iconBitmap = [System.Drawing.Image]::FromFile($tempPng)
$iconHandle = $iconBitmap.GetHicon()
$icon = [System.Drawing.Icon]::FromHandle($iconHandle)

# Save as ICO
$fileStream = New-Object System.IO.FileStream($outputPath, [System.IO.FileMode]::Create)
$icon.Save($fileStream)
$fileStream.Close()

# Cleanup
$icon.Dispose()
$iconBitmap.Dispose()
Remove-Item $tempPng -Force

Write-Host "Icon created successfully at: $outputPath"
Write-Host "Icon file size: $((Get-Item $outputPath).Length) bytes"
