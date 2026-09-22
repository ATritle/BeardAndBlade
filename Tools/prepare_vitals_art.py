"""Slice the generated atlas without painting or altering the artwork."""
from pathlib import Path
from PIL import Image
import sys
root=Path(__file__).resolve().parents[1]
source=root/'ArtSource/Vitals'
source.mkdir(parents=True,exist_ok=True)
dest=root/'Content/Art/V2'
atlas=Image.open(sys.argv[1]).convert('RGBA')
atlas.save(source/'VitalsAtlas.png')
w,h=atlas.size
for name,x,y in [('HUD_EmptyOrb',0,0),('HUD_HealthOrb',1,0),('HUD_StaminaOrb',0,1),('HUD_Potion',1,1)]:
    atlas.crop((x*w//2,y*h//2,(x+1)*w//2,(y+1)*h//2)).save(dest/(name+'.png'))
panel=Image.open(sys.argv[2]).convert('RGBA')
panel.save(source/'HUDPanel.png')
panel.save(dest/'HUD_Panel.png')
print('Prepared five textures',atlas.size,panel.size,'alpha ranges',atlas.getextrema()[3],panel.getextrema()[3])
