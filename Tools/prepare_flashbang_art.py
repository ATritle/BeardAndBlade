"""Normalize generated transparent assets for Canvas runtime; never paint replacement art."""
from pathlib import Path
from PIL import Image
import json, wave
import numpy as np

def isolated_sprite(image):
    """Discard isolated atlas-edge specks without synthesizing any new artwork."""
    pixels=np.array(image);mask=pixels[:,:,3]>40;seen=np.zeros(mask.shape,dtype=bool)
    largest=[]
    for y,x in zip(*np.where(mask)):
        if seen[y,x]:continue
        seen[y,x]=True;stack=[(y,x)];component=[]
        while stack:
            yy,xx=stack.pop();component.append((yy,xx))
            for dy,dx in ((-1,0),(1,0),(0,-1),(0,1),(-1,-1),(-1,1),(1,-1),(1,1)):
                ny,nx=yy+dy,xx+dx
                if 0<=ny<mask.shape[0] and 0<=nx<mask.shape[1] and mask[ny,nx] and not seen[ny,nx]:
                    seen[ny,nx]=True;stack.append((ny,nx))
        if len(component)>len(largest):largest=component
    keep=np.zeros(mask.shape,dtype=bool)
    for y,x in largest:keep[y,x]=True
    pixels[~keep,3]=0
    return Image.fromarray(pixels)

root=Path(__file__).resolve().parents[1]
src=root/'ArtSource/FlashBang'
dest=root/'Content/Art/September'
dest.mkdir(parents=True,exist_ok=True)
sheet=Image.open(src/'SoldierSheet.png').convert('RGBA')
print('Sheet',sheet.size,'alpha',sheet.getchannel('A').getextrema())
# Measured row bounds and stable torso centers; not independent bbox centering,
# which would shift the body when the throwing arm extends.
sx,sy=sheet.width/1280,sheet.height/1280
rows=[(0,310),(310,630),(630,932),(932,1280)]
cols=[(55,370,200),(370,635,500),(635,940,795),(940,1280,1090)]
manifest=[]
for row,(top,bottom) in enumerate(rows):
    for col,(left,right,center) in enumerate(cols):
        im=isolated_sprite(sheet.crop((int(left*sx),int(top*sy),int(right*sx),int(bottom*sy))))
        box=im.getchannel('A').getbbox()
        assert box,(row,col)
        cropped=im.crop(box)
        factor=103/(310*sy)
        reduced=cropped.resize((max(1,round(cropped.width*factor)),max(1,round(cropped.height*factor))),Image.Resampling.NEAREST)
        tile=Image.new('RGBA',(128,128))
        x=round(64+(left*sx+box[0]-center*sx)*factor)
        y=116-reduced.height
        assert x>=0 and x+reduced.width<=128 and y>=0,(row,col,x,y,reduced.size)
        tile.alpha_composite(reduced,(x,y))
        name=f'FlashGuy_{row*4+col}'
        tile.save(dest/f'{name}.png')
        manifest.append(dict(name=name,source_bounds=[left,top,right,bottom],pivot=[64,116]))

fx=Image.open(src/'EffectsSheet.png').convert('RGBA')
for name,region,size in [('FlashGrenade',(90,30,510,655),(128,128)),('FlashBurst',(615,60,1250,710),(256,256)),('BossPortrait_6',(0,660,625,1250),(256,256)),('BossName_6',(580,860,1260,1100),(384,176))]:
    cut=fx.crop(tuple(round(v*(fx.width if i%2==0 else fx.height)/1280) for i,v in enumerate(region)))
    bounds=cut.getchannel('A').getbbox();assert bounds,name
    cut=cut.crop(bounds);cut.thumbnail((size[0]-8,size[1]-8),Image.Resampling.NEAREST)
    tile=Image.new('RGBA',size);tile.alpha_composite(cut,((size[0]-cut.width)//2,(size[1]-cut.height)//2));tile.save(dest/f'{name}.png')

# Original synthesized flash report: short broadband crack, low thump and subdued
# airy tail. No high-pitched tinnitus loop or repeated sharp transient.
rate=44100;t=np.arange(int(rate*.8))/rate;rng=np.random.default_rng(30024)
noise=rng.normal(0,1,len(t))
signal=.5*noise*np.exp(-t*32)+.2*np.sin(2*np.pi*85*t)*np.exp(-t*16)+.09*noise*np.exp(-t*5)
signal*=np.minimum(t/.001,1);signal=np.tanh(signal)*.65
with wave.open(str(root/'Content/Audio/FlashBang.wav'),'wb') as out:
    out.setnchannels(1);out.setsampwidth(2);out.setframerate(rate);out.writeframes((signal*32767).astype('<i2').tobytes())
(src/'manifest.json').write_text(json.dumps(manifest,indent=2))
contact=Image.new('RGBA',(512,512),(36,43,47,255))
for i in range(16):contact.alpha_composite(Image.open(dest/f'FlashGuy_{i}.png'),((i%4)*128,(i//4)*128))
contact.save(src/'AnimationContact.png')
print('FLASH_ART_READY',len(manifest),'frames and 4 UI/FX textures')
