"""Mechanical atlas extraction only; artwork is generated with the built-in image tool."""
from pathlib import Path
from PIL import Image
import sys, shutil
import numpy as np
root=Path(__file__).resolve().parents[1]
out=root/'Content/Art/V2'
source=root/'ArtSource/Freedom';source.mkdir(parents=True,exist_ok=True)
for name,path in zip(['Eagle','Blood','Title','Buttons'],sys.argv[1:]):
    shutil.copy2(path,source/(name+'.png'))
    im=Image.open(path).convert('RGBA');w,h=im.size
    if name in ('Eagle','Blood'):
        for i in range(8):
            cell=im.crop((round(i%4*w/4),round(i//4*h/2),round((i%4+1)*w/4),round((i//4+1)*h/2)))
            side=max(cell.size); padded=Image.new('RGBA',(side,side))
            if name=='Eagle':
                pixels=np.asarray(cell); yy,xx=np.indices(pixels.shape[:2])
                head=(pixels[:,:,0]>185)&(pixels[:,:,1]>185)&(pixels[:,:,2]>160)&(pixels[:,:,3]>128)&(xx>cell.width*.63)
                if not head.any():raise ValueError('Missing eagle head anchor')
                ax=float(np.median(xx[head]));ay=float(np.median(yy[head]))
                padded.paste(cell,(round(side*.66-ax),round(side*.50-ay)))
            else:padded.paste(cell,((side-cell.width)//2,(side-cell.height)//2))
            padded.save(out/f'{name}_{i}.png')
    elif name=='Title':
        im.save(out/'TeaTitle.png')
        im.convert('RGB').resize((1280,800)).save(root/'Content/Splash/Splash.bmp')
    else:
        for i,(a,b) in enumerate([(40,320),(330,610),(620,900),(915,1190)]):
            cell=im.crop((0,round(a*h/1254),w,round(b*h/1254)))
            box=cell.getchannel('A').getbbox()
            if box: cell=cell.crop(box)
            cell.save(out/f'Ornate_{i}.png')
            if i==3:cell.save(root/'Branding/Logo.png')
    print(name,im.size,im.getchannel('A').getextrema())
if len(sys.argv)>5:
    shutil.copy2(sys.argv[5],source/'Remains.png')
    im=Image.open(sys.argv[5]).convert('RGBA');w,h=im.size
    for i in range(4):
        cell=im.crop((i%2*w//2,i//2*h//2,(i%2+1)*w//2,(i//2+1)*h//2))
        box=cell.getchannel('A').getbbox()
        if box:cell=cell.crop(box)
        side=max(cell.size)+16;padded=Image.new('RGBA',(side,side))
        padded.paste(cell,((side-cell.width)//2,(side-cell.height)//2));padded.save(out/f'Remains_{i}.png')
