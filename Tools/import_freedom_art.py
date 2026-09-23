import unreal
import wave
from pathlib import Path
root=Path(unreal.Paths.project_dir());tasks=[]
names=[f'Eagle_{i}' for i in range(8)]+[f'Blood_{i}' for i in range(8)]+['Ornate_2','TeaTitle']
names += [f'Remains_{i}' for i in range(4)]
for name in names:
    task=unreal.AssetImportTask();task.filename=str(root/'Content/Art/V2'/(name+'.png'))
    task.destination_path='/Game/Art/V2';task.automated=True;task.replace_existing=True;task.save=True;tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for task in tasks:
    assert task.imported_object_paths,task.filename
    for path in task.imported_object_paths:
        texture=unreal.load_asset(path)
        texture.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        texture.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property('never_stream',True)
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
unreal.log('FREEDOM_ART_IMPORT_COMPLETE')
audio=[]
for name in ['EagleScreech']:
    with wave.open(str(root/'Content/Audio'/(name+'.wav'))) as wav:
        if wav.getnframes()==0:
            unreal.log_warning(name+' has no audio; skipped. Generate a valid recording before importing.')
            if unreal.EditorAssetLibrary.does_asset_exist('/Game/Audio/'+name): unreal.EditorAssetLibrary.delete_asset('/Game/Audio/'+name)
            continue
    task=unreal.AssetImportTask();task.filename=str(root/'Content/Audio'/(name+'.wav'))
    task.destination_path='/Game/Audio';task.automated=True;task.replace_existing=True;task.save=True;audio.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(audio)
for task in audio: assert task.imported_object_paths,task.filename
