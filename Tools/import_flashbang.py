import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
files=list((root/'Content/Art/September').glob('Flash*.png'))+list((root/'Content/Art/September').glob('Boss*_6.png'))+[root/'Content/Art/V2/Arena5.png',root/'Content/Audio/FlashBang.wav']
for path in files:
    task=unreal.AssetImportTask();task.filename=str(path)
    task.destination_path='/Game/'+str(path.parent.relative_to(root/'Content')).replace('\\','/')
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assert task.imported_object_paths,path
    asset=unreal.load_asset(task.imported_object_paths[0])
    if path.suffix=='.png':
        asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        asset.set_editor_property('never_stream',True)
    else:asset.set_editor_property('looping',False)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('FLASH_IMPORT_COMPLETE count='+str(len(files)))
