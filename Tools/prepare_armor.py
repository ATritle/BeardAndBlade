"""Split generated, registered color atlases. Alpha comes from the original pose in UE's armor material."""
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
names=[f'{s}{g}_{r}_{f}' for s in ('Walk','Attack') for g in ('Cardinal','Diagonal') for r in range(4) for f in range(6)]
for armor in ('Sentinel','Verdant','Warden'):
    sheet=Image.open(root/'ArtSource'/f'Armor{armor}.png').convert('RGBA')
    assert sheet.size==(1536,1024),sheet.size
    for i,name in enumerate(names):
        x,y=(i%12)*128,(i//12)*128
        sheet.crop((x,y,x+128,y+128)).save(root/'Content/Art/V2'/f'{armor}_{name}.png')
