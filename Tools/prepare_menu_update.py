from pathlib import Path
from PIL import Image
import sys,shutil
root=Path(__file__).resolve().parents[1];out=root/'Content/Art/V2'
src=root/'ArtSource/MenuPolish';src.mkdir(parents=True,exist_ok=True)
shutil.copy2(sys.argv[1],src/'Buttons.png')
im=Image.open(sys.argv[1]).convert('RGBA');w,h=im.size
names=['BEGIN_DESCENT','RESUME_DESCENT','HOW_TO_PLAY','NEW_RUN','EXIT','BACK']
for i,name in enumerate(names):
    cell=im.crop((i%2*w//2,i//2*h//3,(i%2+1)*w//2,(i//2+1)*h//3))
    box=cell.getchannel('A').getbbox()
    assert box,name
    cell.crop(box).save(out/('Menu_'+name+'.png'))
eagle=Image.open(out/'Eagle_2.png').convert('RGBA')
eagle.crop(eagle.getchannel('A').getbbox()).save(out/'HUDEagle.png')
