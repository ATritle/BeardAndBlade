"""Mechanical image conversion for Windows branding (Pillow required)."""
from pathlib import Path
import sys
from PIL import Image
root = Path(__file__).resolve().parents[1]
brand = root / "Branding"
brand.mkdir(exist_ok=True)
for source, name in zip(sys.argv[1:], ["Icon.png", "Logo.png"]):
    image = Image.open(source).convert("RGBA")
    image.save(brand / name)
icon = Image.open(brand / "Icon.png")
target = root / "Build/Windows"
target.mkdir(parents=True, exist_ok=True)
icon.save(target / "Application.ico", sizes=[(n,n) for n in (16,24,32,48,64,128,256)])
splash = root / "Content/Splash"
splash.mkdir(parents=True, exist_ok=True)
logo = Image.open(brand / "Logo.png").convert("RGB")
logo.thumbnail((720,480), Image.Resampling.LANCZOS)
logo.save(splash / "Splash.bmp")
assert len(Image.open(target / "Application.ico").ico.sizes()) == 7
print("Windows icon (7 resolutions), logo and launch splash prepared.")
