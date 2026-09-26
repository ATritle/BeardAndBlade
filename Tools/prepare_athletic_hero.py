"""Mechanical atlas extraction preserving generated alpha and native pixel detail."""
from prepare_campaign_art import foreground,gutter,body_bounds
from pathlib import Path
from PIL import Image,ImageDraw
import numpy as np,json
root=Path(__file__).resolve().parents[1]
src=root/'ArtSource/HeroAthletic';dst=root/'Content/Art/V2'
manifest=[]
for state,cols,rows in [('WalkCardinal',6,4),('WalkDiagonal',6,4),('AttackCardinal',6,4),('AttackDiagonal',6,4),('Roll',8,4),('Idle',4,2)]:
    im=Image.open(src/f'{state}.png').convert('RGBA');mask=np.array(im.getchannel('A'))>128
    ys=[0]+[gutter(mask.sum(axis=1),round(r*im.height/rows),round(im.height/rows*.18)) for r in range(1,rows)]+[im.height]
    cells=[]
    for r in range(rows):
        band=mask[ys[r]:ys[r+1]]
        xs=[0]+[gutter(band.sum(axis=0),round(c*im.width/cols),round(im.width/cols*.18)) for c in range(1,cols)]+[im.width]
        for c in range(cols):
            column=mask[:,xs[c]:xs[c+1]].sum(axis=1)
            y0=0 if r==0 else gutter(column,ys[r],round(im.height/rows*.15))
            y1=im.height if r==rows-1 else gutter(column,ys[r+1],round(im.height/rows*.15))
            cell=im.crop((xs[c],y0,xs[c+1],y1));m=np.array(cell.getchannel('A'))>128
            yy,xx=np.where(m);assert len(xx)>100,(state,r,c)
            box=body_bounds(m)
            assert box[3]-box[1]>30,(state,r,c,'invalid silhouette')
            cells.append((r,c,cell.crop(box)))
    # Uniform scale across a sheet, retaining relative crouch/stride sizes.
    heights=[f.height for _,_,f in cells]
    factor=min(288/float(np.median(heights)) if state!='Roll' else 288/max(heights),336/max(f.width for _,_,f in cells),330/max(heights))
    review=Image.new('RGB',(cols*384,rows*408),(32,38,42));draw=ImageDraw.Draw(review)
    for r,c,f in cells:
        f=f.resize((round(f.width*factor),round(f.height*factor)),Image.Resampling.NEAREST)
        frame=Image.new('RGBA',(384,384));frame.paste(f,((384-f.width)//2,348-f.height))
        outrow=([3,0,1,2][r] if state=='Roll' else r)
        name=f'Athletic_{state}_{outrow}_{c}' if state!='Idle' else f'Athletic_Idle_{r*cols+c}'
        frame.save(dst/f'{name}.png');manifest.append(name)
        # Review only: show sprite against a dark board, plus normalized socket-coordinate grid.
        alpha=frame.getchannel('A')
        review.paste(frame,(c*384,r*408),alpha)
        for x in range(0,384,48):draw.line((c*384+x,r*408,c*384+x,r*408+384),fill=(52,58,62))
        for y in range(0,384,48):draw.line((c*384,r*408+y,c*384+384,r*408+y),fill=(52,58,62))
        draw.text((c*384+4,r*408+385),name,fill='white')
    review.save(src/f'{state}Review.png')
# Source idle SW accidentally faces SE: use the matching left-facing counterpart.
Image.open(dst/'Athletic_Idle_3.png').transpose(Image.Transpose.FLIP_LEFT_RIGHT).save(dst/'Athletic_Idle_5.png')
(src/'manifest.json').write_text(json.dumps(manifest,indent=2))
print('Prepared',len(manifest),'384px athletic hero frames')
