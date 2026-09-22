import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
tasks=[]
for name in ['HUD_EmptyOrb','HUD_HealthOrb','HUD_StaminaOrb','HUD_Potion','HUD_Panel']:
    t=unreal.AssetImportTask()
    t.filename=str(root/'Content/Art/V2'/(name+'.png'))
    t.destination_path='/Game/Art/V2'
    t.automated=True
    t.replace_existing=True
    t.save=True
    tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    assert task.imported_object_paths
    for path in task.imported_object_paths:
        texture=unreal.load_asset(path)
        texture.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property('never_stream',True)
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('VITALS_ART_IMPORT_COMPLETE: 5 textures')
