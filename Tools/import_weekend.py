import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir())
for name in json.loads((root/'ArtSource/weekend_frame_manifest.json').read_text()):
    t=unreal.AssetImportTask();t.filename=str(root/'Content/Art/V2'/f'{name}.png');t.destination_path='/Game/Art/V2'
    t.automated=True;t.replace_existing=True;t.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    a=unreal.load_asset('/Game/Art/V2/'+name);assert a,name
    a.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
    a.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    a.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    a.set_editor_property('never_stream',True)
    a.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    unreal.EditorAssetLibrary.save_loaded_asset(a)
for name in ('MusicEnding','MusicMenu'):
    t=unreal.AssetImportTask();t.filename=str(root/'Content/Audio'/f'{name}.wav');t.destination_path='/Game/Audio'
    t.automated=True;t.replace_existing=True;t.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    a=unreal.load_asset('/Game/Audio/'+name);assert a
    a.set_editor_property('looping',True);unreal.EditorAssetLibrary.save_loaded_asset(a)
unreal.log('WEEKEND_IMPORT_COMPLETE')
