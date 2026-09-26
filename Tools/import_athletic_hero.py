import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
names=json.loads((root/'ArtSource/HeroAthletic/manifest.json').read_text())
for name in names:
    task=unreal.AssetImportTask();task.filename=str(root/'Content/Art/V2'/f'{name}.png')
    task.destination_path='/Game/Art/V2';task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    t=unreal.load_asset('/Game/Art/V2/'+name);assert t,name
    for key,value in [('filter',unreal.TextureFilter.TF_NEAREST),('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS),('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON),('never_stream',True),('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)]:t.set_editor_property(key,value)
    unreal.EditorAssetLibrary.save_loaded_asset(t)
unreal.log(f'ATHLETIC_HERO_IMPORT_COMPLETE {len(names)} frames')
