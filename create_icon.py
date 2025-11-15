#!/usr/bin/env python3
from PIL import Image
import sys

def create_icon(png_path, ico_path):
    """Convert PNG to proper ICO with multiple sizes"""
    try:
        # Open the PNG image
        img = Image.open(png_path)
        
        # Convert RGBA to RGB with white background if needed
        if img.mode == 'RGBA':
            # Create a white background
            background = Image.new('RGB', img.size, (255, 255, 255))
            background.paste(img, mask=img.split()[3])  # Use alpha channel as mask
            img = background
        elif img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Common icon sizes for Windows
        sizes = [(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)]
        
        # Create list of resized images
        icon_images = []
        for size in sizes:
            resized = img.resize(size, Image.Resampling.LANCZOS)
            icon_images.append(resized)
        
        # Save as ICO with all sizes
        icon_images[0].save(ico_path, format='ICO', sizes=[(img.width, img.height) for img in icon_images], append_images=icon_images[1:])
        
        print(f"Successfully created icon: {ico_path}")
        return True
        
    except Exception as e:
        print(f"Error creating icon: {e}")
        return False

if __name__ == "__main__":
    png_file = "LOGO/pdEMU_LOGO.png"
    ico_file = "pdemu.ico"
    
    create_icon(png_file, ico_file)
