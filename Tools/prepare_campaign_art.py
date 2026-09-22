"""Split and normalize chroma-key atlases for the Unreal keying material.
No background painting: M_KeySprite performs the key at render time.
"""
from pathlib import Path
from PIL import Image
import numpy as np
import json
root=Path(__file__).resolve().parents[1]
dst=root/'Content/Art/V2'; src=root/'ArtSource'
manifest=[]
def foreground(im):
    a=np.asarray(im.convert('RGB')).astype('int16')
    return np.minimum(a[:,:,0],a[:,:,2])-a[:,:,1]<110

def gutter(counts, expected, radius):
    lo=max(1,expected-radius); hi=min(len(counts)-1,expected+radius)
    values=counts[lo:hi]
    candidates=np.flatnonzero(values==values.min())+lo
    # Prefer the center of a contiguous empty gutter, not its first pixel.
    groups=np.split(candidates,np.where(np.diff(candidates)>1)[0]+1)
    group=min(groups,key=lambda g:abs(float(g.mean())-expected)-len(g)*.2)
    return int(round(float(group.mean())))

def body_bounds(mask):
    # Bounds only: discard disconnected neighboring-cell fragments when cropping.
    remaining=set(zip(*np.where(mask)))
    largest=[]
    while remaining:
        seed=remaining.pop(); stack=[seed]; component=[seed]
        while stack:
            y,x=stack.pop()
            for dy,dx in [(-1,0),(1,0),(0,-1),(0,1),(-1,-1),(-1,1),(1,-1),(1,1)]:
                q=(y+dy,x+dx)
                if q in remaining: remaining.remove(q); stack.append(q); component.append(q)
        if len(component)>len(largest): largest=component
    yy,xx=zip(*largest)
    return min(xx),min(yy),max(xx)+1,max(yy)+1
def split(source,prefix,base=0,roll=False):
    im=Image.open(src/source).convert('RGB')
    assert im.size==(1536,1024),(source,im.size)
    corner=im.getpixel((0,0)); assert corner[0]>180 and corner[2]>180 and corner[1]<70,(source,'not chroma keyed',corner)
    mask=foreground(im)
    # Generated atlases have visual gutters, not exact equal-height rows.
    ys=[0]+[gutter(mask.sum(axis=1),y,80) for y in [256,512,768]]+[1024]
    for row in range(4):
        frames=[]
        band=mask[ys[row]+10:ys[row+1]-10,:]
        xs=[0]+[gutter(band.sum(axis=0),x,30) for x in range(192,1536,192)]+[1536]
        for col in range(8):
            # Effect-only impact cells do not replace the creature body.
            actual=5 if not roll and col==6 else col
            x0,x1=xs[actual],xs[actual+1]
            column=mask[:,x0:x1].sum(axis=1)
            y0=0 if row==0 else gutter(column,ys[row],35)
            y1=1024 if row==3 else gutter(column,ys[row+1],35)
            cell=im.crop((x0,y0,x1,y1))
            cell_mask=foreground(cell)
            yy,xx=np.where(cell_mask)
            assert len(xx)>80,(source,row,col,'empty')
            frames.append(cell.crop(body_bounds(cell_mask)))
        scale=min(96/max(f.height for f in frames),112/max(f.width for f in frames))
        for col,f in enumerate(frames):
            f=f.resize((max(1,round(f.width*scale)),max(1,round(f.height*scale))),Image.Resampling.NEAREST)
            out=Image.new('RGB',(128,128),(255,0,255));out.paste(f,((128-f.width)//2,116-f.height))
            name=f'{prefix}_{row if roll else base+row}_{col}'
            out.save(dst/(name+'.png'));manifest.append(name)
if __name__=='__main__':
    for i in range(6): split(f'MatteRoster{i}.png','Creature',i*4)
    split('BossesReflow.png','Creature',24)
    split('KeyRoll.png','Roll',roll=True)
    for armor in ['Sentinel','Verdant','Warden']:
        if (src/f'Roll{armor}.png').exists(): split(f'Roll{armor}.png',armor+'_Roll',roll=True)
    for i in range(1,4): Image.open(src/f'ExpansionArena{i}.png').save(dst/f'Arena{i}.png')
    Image.open(src/'TitleFinal.png').save(dst/'TitleFinal.png')
    (src/'campaign_frame_manifest.json').write_text(json.dumps(manifest,indent=2))
    print('Prepared',len(manifest),'chroma-key frames; 3 backgrounds and title image.')
