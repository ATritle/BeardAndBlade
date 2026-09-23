import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir());tasks=[]
for name in [f'Loot_{i}' for i in range(48)]+['InventoryFrame']:
    t=unreal.AssetImportTask();t.filename=str(root/'Content/Art/V2'/(name+'.png'));t.destination_path='/Game/Art/V2'
    t.automated=True;t.replace_existing=True;t.save=True;tasks.append(t)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for t in tasks:
    assert t.imported_object_paths,t.filename
    for path in t.imported_object_paths:
        tex=unreal.load_asset(path);tex.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        tex.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        tex.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        tex.set_editor_property('never_stream',True);unreal.EditorAssetLibrary.save_loaded_asset(tex)
unreal.log('LOOT_ART_IMPORT_COMPLETE')
