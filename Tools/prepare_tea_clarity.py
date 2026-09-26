"""Preserve source atlas detail and the original normalized FX placement."""
from prepare_campaign_art import *
im=Image.open(src/'TeaFXKey.png').convert('RGB'); mask=foreground(im)
ys=[0]+[gutter(mask.sum(axis=1),r*im.height//4,60) for r in range(1,4)]+[im.height]
for row in range(4):
    y0,y1=ys[row:row+2]
    xs=[0]+[gutter(mask[y0:y1].sum(axis=0),c*im.width//4,30) for c in range(1,4)]+[im.width]
    for col in range(4):
        cell=im.crop((xs[col],y0,xs[col+1],y1)); yy,xx=np.where(foreground(cell))
        f=cell.crop((xx.min(),yy.min(),xx.max()+1,yy.max()+1))
        s=min(336/f.width,336/f.height)
        f=f.resize((max(1,round(f.width*s)),max(1,round(f.height*s))),Image.Resampling.NEAREST)
        out=Image.new('RGB',(384,384),(255,0,255));out.paste(f,((384-f.width)//2,(384-f.height)//2))
        name=f'TeaFX_{row*4+col}';out.save(dst/f'{name}.png');manifest.append(name)
(src/'tea_clarity_manifest.json').write_text(json.dumps(manifest))
print('Prepared',len(manifest),'high-resolution FX textures')
