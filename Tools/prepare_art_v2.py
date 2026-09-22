"""Slice generated production sheets into isolated, foot-aligned UE textures.
No fixed UV grid sampling at runtime: every frame owns its own transparent image.
"""
from pathlib import Path
import json
import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'ArtSource'
DST = ROOT / 'Content/Art/V2'
DST.mkdir(parents=True, exist_ok=True)
manifest = []

def boundaries(counts, count):
    # Find actual transparent gutters near each expected boundary, not uniform cuts.
    n = len(counts)
    bounds = [0]
    for i in range(1, count):
        lo, hi = int((i-.28)*n/count), int((i+.28)*n/count)
        window = counts[lo:hi]
        minimum = window.min()
        choices = np.where(window == minimum)[0] + lo
        mid = int(choices[np.argmin(abs(choices-i*n/count))])
        if minimum > 3:
            raise ValueError(f'No clear gutter at {i}/{count}: {minimum} occupied pixels')
        bounds.append(mid)
    return bounds+[n]

def extract(name, cols, rows, prefix, height=96, size=128):
    image = Image.open(SRC / (name+'.png')).convert('RGBA')
    alpha = np.array(image.getchannel('A'))
    yb = boundaries((alpha>24).sum(axis=1), rows)
    crops = []
    for row in range(rows):
        xb = boundaries((alpha[yb[row]:yb[row+1]]>24).sum(axis=0), cols)
        for col in range(cols):
            crop = image.crop((xb[col],yb[row],xb[col+1],yb[row+1]))
            box = crop.getchannel('A').point(lambda x: 255 if x>24 else 0).getbbox()
            if not box: raise ValueError('Empty frame')
            crops.append((row,col,crop.crop(box)))
    # One shared scale preserves relative pose/body proportions within each sheet.
    scale = min(height/max(c.height for _,_,c in crops), (size-16)/max(c.width for _,_,c in crops))
    for row,col,crop in crops:
        crop = crop.resize((max(1,round(crop.width*scale)),max(1,round(crop.height*scale))),Image.Resampling.NEAREST)
        frame = Image.new('RGBA',(size,size))
        frame.alpha_composite(crop,((size-crop.width)//2,size-12-crop.height))
        fname = f'{prefix}_{row}_{col}'
        frame.save(DST/(fname+'.png'))
        assert not frame.getchannel('A').crop((0,0,size,4)).getbbox()
        assert not frame.getchannel('A').crop((0,size-4,size,size)).getbbox()
        manifest.append({'name':fname,'size':size,'baseline':size-12})

for state in ['Walk','Attack']:
    for group in ['Cardinal','Diagonal']:
        extract(state+group,6,4,state+group)
extract('Loot',3,3,'Item',96)
extract('Boss',1,1,'Boss',202,256)
extract('EnemyWalk',4,4,'EnemyWalk',96)
extract('EnemyAttack',4,4,'EnemyAttack',96)
extract('BossMotion',4,2,'BossMotion',202,256)
for source,target in [('ChestSprite','Chest'),('PortalSprite','Portal'),('SkeletonSprite','Skeleton'),('SkeletonAttack','SkeletonAttack'),('SkeletonHit','SkeletonHit')]:
    image=Image.open(ROOT/'Content/Art'/f'{source}.png').convert('RGBA')
    image=image.crop(image.getchannel('A').getbbox())
    image.thumbnail((112,104),Image.Resampling.NEAREST)
    frame=Image.new('RGBA',(128,128));frame.alpha_composite(image,((128-image.width)//2,116-image.height));frame.save(DST/(target+'.png'))
(ROOT/'ArtSource/frame_manifest.json').write_text(json.dumps(manifest,indent=2))
# Human-review contact sheet on checkerboard, all 96 frames with labels.
review=Image.new('RGB',(12*128,8*148),(36,44,50));draw=ImageDraw.Draw(review)
for i,entry in enumerate(manifest[:96]):
    x,y=(i%12)*128,(i//12)*148
    review.paste(Image.open(DST/(entry['name']+'.png')),(x,y),Image.open(DST/(entry['name']+'.png')))
    draw.text((x+3,y+128),entry['name'].replace('Cardinal','C').replace('Diagonal','D'),fill='white')
review.save(ROOT/'ArtSource/FrameReview.png')
print(f'Validated {len(manifest)} isolated frames; transparent padding on all frame edges.')
