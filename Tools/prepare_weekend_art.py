"""Re-extract original art at higher resolution; no generated or painted detail."""
from prepare_campaign_art import *
for i in range(6): split(f'MatteRoster{i}.png','Creature',i*4,size=256)
im=Image.open(src/'FinanceKey.png').convert('RGB');mask=foreground(im)
ys=[0,gutter(mask.sum(axis=1),im.height//2,60),im.height]
frames=[]
for row in range(2):
    y0,y1=ys[row:row+2]
    xs=[0]+[gutter(mask[y0:y1].sum(axis=0),c*im.width//4,30) for c in range(1,4)]+[im.width]
    for col in range(4):
        cell=im.crop((xs[col],y0,xs[col+1],y1));frames.append(cell.crop(body_bounds(foreground(cell))))
scale=min(288/max(f.height for f in frames),336/max(f.width for f in frames))
for i,f in enumerate(frames):
    f=f.resize((round(f.width*scale),round(f.height*scale)),Image.Resampling.NEAREST)
    out=Image.new('RGB',(384,384),(255,0,255));out.paste(f,((384-f.width)//2,348-f.height))
    name=f'Finance_{i}';out.save(dst/f'{name}.png');manifest.append(name)
(src/'weekend_frame_manifest.json').write_text(json.dumps(manifest))
print('Re-extracted',len(manifest),'frames from original sheets')
