"""Inspected source-atlas extraction. Originals are immutable; exports carry crop metadata.

Alpha threshold removes the generated low-alpha colored backdrop. No chromakey.
Grid counts are search hints only: boundaries seek actual transparent gutters.
Chest row bounds were inspected independently (not the rejected preview grid).
"""
from pathlib import Path
import json
import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'ArtSource'
OUT = ROOT / 'Content/Art/September'
QA = ROOT / 'ArtSource/SeptemberQA'
OUT.mkdir(parents=True, exist_ok=True)
QA.mkdir(parents=True, exist_ok=True)
manifest = {}

def clean(path):
    a = np.array(Image.open(path).convert('RGBA'))
    # Source opaque bodies peak at alpha 252. Low-alpha haze is not intended art.
    a[:, :, 3] = np.where(a[:, :, 3] >= 190, 255, 0)
    a[a[:, :, 3] == 0, :3] = 0
    return Image.fromarray(a)

def cuts(signal, count):
    n=len(signal); result=[0]
    for i in range(1,count):
        lo=round((i-.25)*n/count); hi=round((i+.25)*n/count)
        # Prefer the middle of a clear gutter, not its first pixel.
        window=signal[lo:hi]; best=np.flatnonzero(window==window.min())+lo
        result.append(int(best[np.argmin(abs(best-i*n/count))]))
    return result+[n]

def export(name, im, box, canvas, baseline=True):
    part=im.crop(tuple(map(int,box))); bounds=part.getbbox()
    if not bounds:
        if not name.startswith('LootEffect_'): raise ValueError(name+' empty')
        frame=Image.new('RGBA',canvas); frame.save(OUT/(name+'.png'))
        manifest[name]={'crop':list(map(int,box)),'canvas':canvas,'empty':True}
        return frame
    part=part.crop(bounds)
    if part.width>canvas[0]-16 or part.height>canvas[1]-16:
        raise ValueError(f'{name}: content {part.size} exceeds padded canvas {canvas}')
    x=(canvas[0]-part.width)//2; y=canvas[1]-12-part.height if baseline else (canvas[1]-part.height)//2
    if name.startswith(('Twister_','Mack_')):
        foot=np.array(part)[:,:,3]>0
        if name.startswith('Twister_'):
            # Muzzle flashes can hang below the funnel. Find the tip only near
            # the source cell's body axis, not from the complete sprite bbox.
            axis=(box[2]-box[0])/2-bounds[0]
            foot[:,:max(0,int(axis-30))]=False; foot[:,min(part.width,int(axis+30)):]=False
            rows=np.where(foot.any(axis=1))[0]
            tip=int(rows[-1])
            xx=np.where(foot[max(0,tip-3):tip+1])[1]
            x=round(canvas[0]/2-float(np.median(xx))); y=canvas[1]-24-tip
        else:
            xx=np.where(foot[-max(4,part.height//16):])[1]
            x=round(canvas[0]/2-float(np.median(xx)))
        x=max(8,min(canvas[0]-part.width-8,x))
    frame=Image.new('RGBA',canvas); frame.paste(part,(x,y))
    frame.save(OUT/(name+'.png'))
    floor_margin = 24 if name.startswith('Twister_') else 12
    manifest[name]={'crop':[int(box[0]+bounds[0]),int(box[1]+bounds[1]),int(box[0]+bounds[2]),int(box[1]+bounds[3])], 'canvas':canvas,'pivot':[canvas[0]/2,canvas[1]-floor_margin] if baseline else [canvas[0]/2,canvas[1]/2]}
    return frame

def sheet(path,prefix,cols,rows,canvas,baseline=False,row_bounds=None):
    im=clean(path); mask=np.array(im)[:,:,3]>0
    ys=row_bounds or cuts(mask.sum(axis=1),rows)
    frames=[]
    for r in range(rows):
        xs=cuts(mask[ys[r]:ys[r+1]].sum(axis=0),cols)
        for c in range(cols):
            frames.append(export(f'{prefix}_{r*cols+c}',im,(xs[c],ys[r],xs[c+1],ys[r+1]),canvas,baseline))
    preview(frames,prefix,cols)
    return frames

def preview(frames,name,cols):
    tile=160; rows=(len(frames)+cols-1)//cols
    contact=Image.new('RGB',(cols*tile,rows*(tile+20)),(34,43,49)); d=ImageDraw.Draw(contact)
    for i,f in enumerate(frames):
        f=f.copy(); f.thumbnail((tile-8,tile-8),Image.Resampling.NEAREST)
        x=i%cols*tile; y=i//cols*(tile+20)
        if i%2: d.rectangle((x,y,x+tile,y+tile),fill=(210,215,220))
        contact.paste(f,(x+(tile-f.width)//2,y+tile-f.height),f);d.text((x+4,y+tile+2),str(i),fill='white')
    contact.save(QA/(name+'.png'))

dirs=['north','northeast','east','southeast','south','southwest','west','northwest']
for d,direction in enumerate(dirs):
    sheet(SRC/f'Bosses/BigMack/directional-v2/big-mack-{direction}.png',f'Mack_{d}',4,4,(384,384),True)
    sheet(SRC/f'Bosses/Twister/directional-v1/twister-{direction}.png',f'Twister_{d}',6,4,(384,384),True)
# Southwest throw source turns right. Keep its dedicated idle/hop; correct only
# these four throw poses using the matching southeast pose mirrored to the left.
for f in range(12,16):
    Image.open(OUT/f'Mack_3_{f}.png').transpose(Image.Transpose.FLIP_LEFT_RIGHT).save(OUT/f'Mack_5_{f}.png')
    manifest[f'Mack_5_{f}']['correction']='mirrored southeast throw: southwest source facing drift'
sheet(SRC/'LootPresentation/v2/chest-opening-12-frames.png','RewardChest',4,3,(352,352),True,[0,330,680,1086])
sheet(SRC/'LootPresentation/v2/rarity-glows-6-frame-loops.png','RarityGlow',6,5,(320,320),True)
sheet(SRC/'LootPresentation/v2/loot-release-landing-pickup-8-frames.png','LootEffect',8,3,(256,320))
sheet(SRC/'UI/StatusEffects/v1/status-effects-8-frame-atlas.png','Status',8,5,(208,240))
for file,prefix,cols,rows,canvas in [('burger-splat','BurgerSplat',4,3,(400,384)),('landing-shockwave','MackLanding',4,3,(416,384)),('lettuce-cheese-debris','FoodDebris',6,4,(288,288))]:
    sheet(SRC/f'Bosses/BigMack/combat-effects-v1/{file}.png',prefix,cols,rows,canvas)
# Separate measured UI components, with exact alpha clearing of fill openings.
for version,file,ys,xs,start in [('v1','current-bosses-ui.png',[0,253,508,753,1024],[0,305,790,1536],0),('v2','big-mack-twister-ui.png',[0,360,724],[0,430,1110,2172],4)]:
    im=clean(SRC/f'UI/BossUI/{version}/{file}')
    for row in range(len(ys)-1):
        for col,kind in enumerate(['Portrait','Name','Border']):
            name=f'Boss{kind}_{start+row}'
            export(name,im,(xs[col],ys[row],xs[col+1],ys[row+1]),(xs[col+1]-xs[col]+24,ys[row+1]-ys[row]+24),False)
# Finance source portrait materially changes the established face. Preserve his
# actual sprite identity inside the uploaded frame (not the new villain face).
portrait=Image.open(OUT/'BossPortrait_0.png').convert('RGBA')
original=np.array(Image.open(ROOT/'Content/Art/V2/Finance_0.png').convert('RGBA'))
key=(original[:,:,0]>180)&(original[:,:,2]>180)&(original[:,:,1]<100)
original[key]=0
head=Image.fromarray(original).crop((47,17,87,68)).resize((170,216),Image.Resampling.NEAREST)
ImageDraw.Draw(portrait).rectangle((57,32,272,245),fill=(25,30,38,255))
portrait.alpha_composite(head,(79,30));portrait.save(OUT/'BossPortrait_0.png')
# Actual safe health openings, measured in source coordinates, mapped to each
# padded border export. Clear only the opening, never the ornament.
holes=[(925,132,1429,176),(936,389,1407,430),(935,632,1420,674),(930,856,1427,900),(1280,192,1980,237),(1270,521,1980,564)]
fill=[]
for i,hole in enumerate(holes):
    m=manifest[f'BossBorder_{i}']; x0,y0,x1,y1=m['crop']; w,h=m['canvas']
    ox=(w-(x1-x0))//2-x0;oy=(h-(y1-y0))//2-y0
    b=[hole[0]+ox,hole[1]+oy,hole[2]+ox,hole[3]+oy]
    im=Image.open(OUT/f'BossBorder_{i}.png'); ImageDraw.Draw(im).rectangle(b,fill=(0,0,0,0));im.save(OUT/f'BossBorder_{i}.png')
    fill.append([b[0]/w,b[1]/h,(b[2]-b[0])/w,(b[3]-b[1])/h])
(OUT/'health-openings.json').write_text(json.dumps(fill,indent=2))
(OUT/'frames.json').write_text(json.dumps(manifest,indent=2))
print(f'Exported {len(manifest)} frames to {OUT}')
