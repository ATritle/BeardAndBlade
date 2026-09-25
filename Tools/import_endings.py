import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
for name in ('Death','Victory'):
    task=unreal.AssetImportTask()
    task.filename=str(root/'ArtSource/Endings'/f'{name}.png')
    task.destination_path='/Game/Art/Endings';task.destination_name=f'Ending{name}'
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assert task.imported_object_paths, name
    asset=unreal.load_asset(task.imported_object_paths[0])
    asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
    asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    asset.set_editor_property('never_stream',True)
    asset.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('ENDING_ART_IMPORTED 2 scenes')
