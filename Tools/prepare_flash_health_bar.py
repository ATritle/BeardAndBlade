"""Crop/size the generated border and measure its transparent health opening."""
from pathlib import Path
from PIL import Image
import numpy as np
import json
root=Path(__file__).resolve().parents[1]
im=Image.open(root/'ArtSource/FlashBang/MilitaryHealthBar-source.png').convert('RGBA')
im=im.crop(im.getchannel('A').point(lambda a:255 if a>100 else 0).getbbox())
im.thumbnail((650,150),Image.Resampling.NEAREST)
tile=Image.new('RGBA',(666,180));tile.alpha_composite(im,((666-im.width)//2,(180-im.height)//2))
alpha=np.array(tile)[:,:,3]
# Central enclosed hole, bounded by opaque inner rails in both axes.
cx,cy=333,90
assert alpha[cy,cx]<10,'Generated center must be transparent'
x0=cx
while alpha[cy,x0-1]<100:x0-=1
x1=cx
while alpha[cy,x1+1]<100:x1+=1
y0=cy
while alpha[y0-1,cx]<100:y0-=1
y1=cy
while alpha[y1+1,cx]<100:y1+=1
tile.save(root/'Content/Art/September/BossBorder_6.png')
data=dict(canvas=[666,180],opening=[x0,y0,x1+1,y1+1],uv=[x0/666,y0/180,(x1+1-x0)/666,(y1+1-y0)/180])
(root/'ArtSource/FlashBang/health_bar_manifest.json').write_text(json.dumps(data,indent=2))
print(data)
