"""Mechanical atlas slicing; backgrounds are keyed by Unreal, never painted here."""
from prepare_campaign_art import *
names=[f'{s}{g}_{r}_{f}' for s in ('Walk','Attack') for g in ('Cardinal','Diagonal') for r in range(4) for f in range(6)]
for outfit in ('Base','Sentinel','Verdant','Warden'):
    sheet=Image.open(src/f'TeaHero{outfit}.png')
    assert sheet.size==(1536,1024)
    for i,name in enumerate(names):
        x,y=i%12*128,i//12*128
        name=f'Tea_{outfit}_{name}'
        sheet.crop((x,y,x+128,y+128)).save(dst/f'{name}.png');manifest.append(name)
    prefix='Tea_Roll' if outfit=='Base' else f'Tea_{outfit}_Roll'
    split(f'TeaRoll{outfit}Key.png',prefix,roll=True)

def icons(source,prefix,rows,body=False):
    im=Image.open(src/source).convert('RGB');mask=foreground(im)
    frames=[]
    ys=[0]+[gutter(mask.sum(axis=1),r*im.height//rows,60) for r in range(1,rows)]+[im.height]
    for row in range(rows):
        y0,y1=ys[row],ys[row+1]
        xs=[0]+[gutter(mask[y0:y1].sum(axis=0),c*im.width//4,30) for c in range(1,4)]+[im.width]
        for col in range(4):
            x0,x1=xs[col],xs[col+1]
            cell=im.crop((x0,y0,x1,y1));m=foreground(cell)
            if body: box=body_bounds(m)
            else:
                yy,xx=np.where(m);box=(xx.min(),yy.min(),xx.max()+1,yy.max()+1)
            frames.append(cell.crop(box))
    scale=min(96/max(f.height for f in frames),112/max(f.width for f in frames))
    for i,f in enumerate(frames):
        s=scale if body else min(112/f.width,112/f.height)
        f=f.resize((max(1,round(f.width*s)),max(1,round(f.height*s))),Image.Resampling.NEAREST)
        out=Image.new('RGB',(128,128),(255,0,255))
        out.paste(f,((128-f.width)//2,116-f.height if body else (128-f.height)//2))
        name=f'{prefix}_{i}';out.save(dst/f'{name}.png');manifest.append(name)
icons('FinanceKey.png','Finance',2,True)
icons('TeaFXKey.png','TeaFX',4)
Image.open(src/'TeaTitle.png').save(dst/'TeaTitle.png');manifest.append('TeaTitle')
(src/'tea_frame_manifest.json').write_text(json.dumps(manifest,indent=2))
print('Prepared',len(manifest),'tea update textures')
