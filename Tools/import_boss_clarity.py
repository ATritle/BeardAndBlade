import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
names=json.loads((root/'ArtSource/boss_clarity_manifest.json').read_text())
for name in names:
    task=unreal.AssetImportTask();task.filename=str(root/'Content/Art/V2'/f'{name}.png')
    task.destination_path='/Game/Art/V2';task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset=unreal.load_asset('/Game/Art/V2/'+name);assert asset,name
    asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
    asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    asset.set_editor_property('never_stream',True)
    asset.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('BOSS_CLARITY_IMPORT_COMPLETE 24 frames')
