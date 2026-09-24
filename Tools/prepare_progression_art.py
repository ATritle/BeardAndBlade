"""Normalize generated atlas gutters and stable per-species scale/foot anchors."""
from pathlib import Path
from PIL import Image, ImageDraw
import numpy as np
import json
root=Path(__file__).resolve().parents[1]
src=root/'ArtSource/Progression';out=root/'Content/Art/Progression'
out.mkdir(parents=True,exist_ok=True)
metadata={}
def clean_fragments(cell):
    data=np.array(cell);mask=data[:,:,3]>45;seen=np.zeros(mask.shape,bool);groups=[]
    for y,x in zip(*np.nonzero(mask)):
        if seen[y,x]:continue
        todo=[(y,x)];seen[y,x]=True;group=[]
        while todo:
            cy,cx=todo.pop();group.append((cy,cx))
            for dy,dx in ((1,0),(-1,0),(0,1),(0,-1),(1,1),(1,-1),(-1,1),(-1,-1)):
                ny,nx=cy+dy,cx+dx
                if 0<=ny<mask.shape[0] and 0<=nx<mask.shape[1] and mask[ny,nx] and not seen[ny,nx]:
                    seen[ny,nx]=True;todo.append((ny,nx))
        groups.append(group)
    largest=max(map(len,groups),default=1)
    for group in groups:
        if len(group)<largest*.008:
            for y,x in group:data[max(0,y-1):y+2,max(0,x-1):x+2,3]=0
    return Image.fromarray(data)
def cuts(profile,n):
    size=len(profile);result=[0]
    for i in range(1,n):
        mid=size*i/n;radius=size/n*.20
        lo,hi=int(mid-radius),int(mid+radius)
        segment=profile[lo:hi]
        candidates=np.flatnonzero(segment==segment.min())+lo
        result.append(int(min(candidates,key=lambda x:abs(x-mid))))
    return result+[size]
for name,base in [('Kitchen',31),('Bunker',37),('Storm',43)]:
    atlas=Image.open(src/(name+'-source.png')).convert('RGBA')
    alpha=np.array(atlas)[:,:,3];assert alpha.min()==0,(name,'Missing transparency')
    ys=cuts((alpha>70).sum(axis=1),6)
    review=Image.new('RGB',(8*132,6*148),(27,34,40));draw=ImageDraw.Draw(review)
    for row in range(6):
        xs=cuts((alpha[ys[row]:ys[row+1]]>70).sum(axis=0),8)
        frames=[];boxes=[]
        for col in range(8):
            box=(xs[col],ys[row],xs[col+1],ys[row+1]);cell=clean_fragments(atlas.crop(box))
            bounds=cell.getchannel('A').point(lambda a:255 if a>50 else 0).getbbox()
            assert bounds,(name,row,col)
            frames.append(cell.crop(bounds));boxes.append([box,bounds])
        factor=min(224/max(f.width for f in frames),216/max(f.height for f in frames))
        for col,frame in enumerate(frames):
            frame=frame.resize((max(1,round(frame.width*factor)),max(1,round(frame.height*factor))),Image.Resampling.NEAREST)
            canvas=Image.new('RGBA',(256,256));canvas.alpha_composite(frame,((256-frame.width)//2,232-frame.height))
            canvas.save(out/f'ThemeEnemy_{base+row}_{col}.png')
            thumb=canvas.resize((128,128),Image.Resampling.NEAREST)
            review.paste(thumb,(col*132,row*148),thumb)
        draw.text((2,row*148+128),f'{base+row}: walk 0-3 / attack 4-7',fill='white')
        metadata[str(base+row)]={'cuts':boxes,'scale':factor}
    review.save(src/(name+'-frames-review.png'))
Image.open(src/'Greaseworks-source.png').convert('RGB').resize((1280,800),Image.Resampling.NEAREST).save(root/'Content/Art/V2/Arena6.png')
fxpath=src/'ThemeFX-source.png'
if fxpath.exists():
    atlas=Image.open(fxpath).convert('RGBA');alpha=np.array(atlas)[:,:,3]
    assert alpha.min()==0,'FX requires transparent alpha'
    ys=cuts((alpha>70).sum(axis=1),3)
    for row in range(3):
        xs=cuts((alpha[ys[row]:ys[row+1]]>70).sum(axis=0),4)
        for col in range(4):
            cell=clean_fragments(atlas.crop((xs[col],ys[row],xs[col+1],ys[row+1])))
            bounds=cell.getchannel('A').point(lambda a:255 if a>35 else 0).getbbox();cell=cell.crop(bounds)
            cell.thumbnail((120,120),Image.Resampling.NEAREST)
            canvas=Image.new('RGBA',(128,128));canvas.alpha_composite(cell,((128-cell.width)//2,(128-cell.height)//2))
            canvas.save(out/f'ThemeFX_{row*4+col}.png')
(src/'crop-metadata.json').write_text(json.dumps(metadata,indent=2))
print('PROGRESSION_ART_READY',len(list(out.glob('*.png'))))
