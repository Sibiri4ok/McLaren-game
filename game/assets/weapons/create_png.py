#!/usr/bin/env python3
"""
Create simple PNG files without PIL - using pypng or raw PNG generation
"""
import struct
import zlib

def create_png(filename, width, height, pixels):
    """
    Create a PNG file from raw pixel data
    pixels should be a list of (r, g, b, a) tuples for each pixel
    """
    def chunk(chunk_type, data):
        """Create a PNG chunk"""
        chunk_data = chunk_type + data
        crc = zlib.crc32(chunk_data) & 0xffffffff
        return struct.pack('>I', len(data)) + chunk_data + struct.pack('>I', crc)
    
    # PNG signature
    signature = b'\x89PNG\r\n\x1a\n'
    
    # IHDR chunk
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 6, 0, 0, 0)  # 6 = RGBA
    ihdr = chunk(b'IHDR', ihdr_data)
    
    # IDAT chunk - image data
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00'  # filter type
        for x in range(width):
            idx = y * width + x
            if idx < len(pixels):
                r, g, b, a = pixels[idx]
                raw_data += bytes([r, g, b, a])
            else:
                raw_data += b'\x00\x00\x00\x00'
    
    compressed_data = zlib.compress(raw_data, 9)
    idat = chunk(b'IDAT', compressed_data)
    
    # IEND chunk
    iend = chunk(b'IEND', b'')
    
    # Write file
    with open(filename, 'wb') as f:
        f.write(signature + ihdr + idat + iend)

# Create pistol-idle.png (32x32)
def create_pistol_idle():
    width, height = 32, 32
    pixels = []
    
    for y in range(height):
        for x in range(width):
            # Simple pistol shape
            r, g, b, a = 0, 0, 0, 0  # transparent by default
            
            # Barrel (horizontal rectangle)
            if 18 <= x < 28 and 12 <= y < 16:
                r, g, b, a = 80, 80, 90, 255
            # Body
            elif 12 <= x < 20 and 10 <= y < 18:
                r, g, b, a = 70, 70, 80, 255
            # Grip
            elif 10 <= x < 16 and 18 <= y < 26:
                # Brown grip
                r, g, b, a = 139, 90, 43, 255
            # Sight
            elif 18 <= x < 19 and 10 <= y < 12:
                r, g, b, a = 50, 50, 60, 255
            
            pixels.append((r, g, b, a))
    
    create_png('/home/danil/McLaren/game/assets/weapons/pistol-idle.png', width, height, pixels)
    print("✓ Created pistol-idle.png")

# Create pistol-shoot.png (4 frames, 128x32)
def create_pistol_shoot():
    width, height = 128, 32
    pixels = []
    
    for y in range(height):
        for x in range(width):
            r, g, b, a = 0, 0, 0, 0
            
            frame = x // 32
            x_in_frame = x % 32
            
            # Draw pistol in each frame
            # Barrel
            if 18 <= x_in_frame < 28 and 12 <= y < 16:
                r, g, b, a = 80, 80, 90, 255
            # Body
            elif 12 <= x_in_frame < 20 and 10 <= y < 18:
                r, g, b, a = 70, 70, 80, 255
            # Grip
            elif 10 <= x_in_frame < 16 and 18 <= y < 26:
                r, g, b, a = 139, 90, 43, 255
            # Sight
            elif 18 <= x_in_frame < 19 and 10 <= y < 12:
                r, g, b, a = 50, 50, 60, 255
            
            # Muzzle flash for first 2 frames
            if frame < 2:
                flash_size = 8 if frame == 0 else 5
                flash_x = 28
                flash_y = 14
                
                dx = x_in_frame - flash_x
                dy = y - flash_y
                dist_sq = dx*dx + dy*dy
                
                if dist_sq < flash_size * flash_size:
                    # Yellow/orange flash
                    if dist_sq < (flash_size - 2) * (flash_size - 2):
                        r, g, b, a = 255, 255, 200, 255
                    else:
                        r, g, b, a = 255, 220, 0, 200
            
            pixels.append((r, g, b, a))
    
    create_png('/home/danil/McLaren/game/assets/weapons/pistol-shoot.png', width, height, pixels)
    print("✓ Created pistol-shoot.png")

# Create bullet.png (16x16)
def create_bullet():
    width, height = 16, 16
    pixels = []
    
    for y in range(height):
        for x in range(width):
            r, g, b, a = 0, 0, 0, 0
            
            # Circle centered at (8, 8) with radius 4
            dx = x - 8
            dy = y - 8
            dist_sq = dx*dx + dy*dy
            
            if dist_sq < 16:  # radius 4
                if dist_sq < 4:  # inner bright circle
                    r, g, b, a = 255, 255, 150, 255
                else:
                    r, g, b, a = 255, 200, 0, 255
            
            pixels.append((r, g, b, a))
    
    create_png('/home/danil/McLaren/game/assets/weapons/bullet.png', width, height, pixels)
    print("✓ Created bullet.png")

# Create bullet-impact.png (4 frames, 64x16)
def create_bullet_impact():
    width, height = 64, 16
    pixels = []
    
    for y in range(height):
        for x in range(width):
            r, g, b, a = 0, 0, 0, 0
            
            frame = x // 16
            x_in_frame = x % 16
            
            # Impact gets smaller each frame
            size = max(0, 12 - frame * 3)
            alpha = max(0, 200 - frame * 50)
            
            dx = x_in_frame - 8
            dy = y - 8
            dist_sq = dx*dx + dy*dy
            
            if dist_sq < size * size and size > 0:
                if dist_sq < (size - 2) * (size - 2):
                    r, g, b, a = 255, 220, 0, min(255, alpha + 55)
                else:
                    r, g, b, a = 255, 150, 0, alpha
            
            pixels.append((r, g, b, a))
    
    create_png('/home/danil/McLaren/game/assets/weapons/bullet-impact.png', width, height, pixels)
    print("✓ Created bullet-impact.png")

if __name__ == '__main__':
    print("Creating weapon assets...")
    create_pistol_idle()
    create_pistol_shoot()
    create_bullet()
    create_bullet_impact()
    print("\n✅ All weapon assets created successfully!")

