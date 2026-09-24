import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
files=list((root/'Content/Art/Progression').glob('*.png'))+[root/'Content/Art/V2/Arena6.png']
for path in files:
    task=unreal.AssetImportTask();task.filename=str(path)
    task.destination_path='/Game/'+str(path.parent.relative_to(root/'Content')).replace('\\','/')
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assert task.imported_object_paths,path
    asset=unreal.load_asset(task.imported_object_paths[0])
    asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
    asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    asset.set_editor_property('never_stream',True)
    asset.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    asset.set_editor_property('lod_bias',0)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('PROGRESSION_IMPORT_COMPLETE count='+str(len(files)))
