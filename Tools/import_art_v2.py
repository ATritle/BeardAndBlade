import unreal
from pathlib import Path
root=Path(unreal.Paths.project_content_dir())/'Art/V2'
tasks=[]
for source in root.glob('*.png'):
    if unreal.EditorAssetLibrary.does_asset_exist('/Game/Art/V2/'+source.stem):
        continue
    task=unreal.AssetImportTask()
    task.filename=str(source.resolve());task.destination_path='/Game/Art/V2'
    task.automated=True;task.replace_existing=True;task.save=True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    for path in task.imported_object_paths:
        texture=unreal.load_asset(path)
        texture.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property('never_stream',True)
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('ART_V2_IMPORT_COMPLETE: '+str(len(tasks))+' textures')
