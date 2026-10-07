#!/usr/bin/env python3
# img2bmp.py - image -> WxH color RGB565 C bitmap
# usage: img2bmp.py image [-w 100] [-H 100] [-b] [-g] [-k]
#   -w/-H  output size
#   -b     byte-swap each pixel (big-endian SPI displays)
#   -g     grayscale
#   -k     keep aspect ratio (letterbox with black)

# Generate with claude

import sys, argparse
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("file")
ap.add_argument("-w", type=int, default=100)
ap.add_argument("-H", type=int, default=100)
ap.add_argument("-b", action="store_true")
ap.add_argument("-g", action="store_true")
ap.add_argument("-k", action="store_true")
a = ap.parse_args()

W, H = a.w, a.H
img = Image.open(a.file).convert("RGB")

if a.k:
    img.thumbnail((W, H), Image.LANCZOS)
    canvas = Image.new("RGB", (W, H), (0, 0, 0))
    canvas.paste(img, ((W - img.width) // 2, (H - img.height) // 2))
    img = canvas
else:
    img = img.resize((W, H), Image.LANCZOS)

if a.g:
    img = img.convert("L").convert("RGB")

def rgb565(r, g, b):
    p = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    if a.b:
        p = ((p & 0xFF) << 8) | (p >> 8)
    return p

px = img.load()
print("#include <stdint.h>\n")
print(f"#define BITMAP_W {W}")
print(f"#define BITMAP_H {H}\n")
print("static const uint16_t bitmap[BITMAP_W * BITMAP_H] = {")
for y in range(H):
    print("    " + ",".join(f"0x{rgb565(*px[x, y]):04X}" for x in range(W)) + ",")
print("};")
