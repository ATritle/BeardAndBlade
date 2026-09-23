from pathlib import Path
from PIL import Image
import shutil,sys
root=Path(__file__).resolve().parents[1]
source=root/'ArtSource/LootUpdate';source.mkdir(parents=True,exist_ok=True)
shutil.copy2(sys.argv[1],source/'EquipmentAtlas.png')
atlas=Image.open(source/'EquipmentAtlas.png').convert('RGBA')
out=root/'Content/Art/V2'
rows=[0,183,355,529,688,846,1024]
for i in range(48):
    r,c=divmod(i,8)
    tile=atlas.crop((c*192,rows[r],(c+1)*192,rows[r+1]))
    box=tile.getchannel('A').point(lambda x:255 if x>140 else 0).getbbox()
    assert box, i
    tile=tile.crop(box)
    tile.thumbnail((100,108),Image.Resampling.NEAREST)
    canvas=Image.new('RGBA',(128,128))
    canvas.alpha_composite(tile,((128-tile.width)//2,112-tile.height))
    canvas.save(out/f'Loot_{i}.png')
if len(sys.argv)>2:
    shutil.copy2(sys.argv[2],source/'InventoryFrame.png')
    shutil.copy2(sys.argv[2],out/'InventoryFrame.png')
