import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
for filename,dest in [(root/'Content/Art/V2/Arena4.png','/Game/Art/V2')]+[(root/f'Content/Audio/Rifle{i}.wav','/Game/Audio') for i in range(3)]:
    t=unreal.AssetImportTask(); t.filename=str(filename);t.destination_path=dest;t.automated=True;t.replace_existing=True;t.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
    assert t.imported_object_paths,filename
    asset=unreal.load_asset(t.imported_object_paths[0])
    if filename.suffix=='.png':
        asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        asset.set_editor_property('never_stream',True)
    else: asset.set_editor_property('looping',False)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('TWISTER_UPDATE_IMPORT_COMPLETE')
