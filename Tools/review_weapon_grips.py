from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
out=Image.new('RGB',(8*256,3*284),(45,52,57));d=ImageDraw.Draw(out)
for i in range(24):
    im=Image.open(root/f'Content/Art/V2/Loot_{i}.png').convert('RGBA').resize((256,256),Image.Resampling.NEAREST)
    x=i%8*256;y=i//8*284
    out.paste(im,(x,y),im)
    d.text((x+5,y+258),str(i),fill='white')
    for n in range(0,128,16):
        d.text((x+n*2,y+2),str(n),fill='yellow')
        d.text((x+2,y+n*2),str(n),fill='yellow')
out.save(root/'ArtSource/SeptemberQA/WeaponGrips.png')
