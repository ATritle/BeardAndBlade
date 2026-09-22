import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_content_dir())/'Audio'
manifest=json.loads((root/'audio_manifest.json').read_text())
tasks=[]
for name in manifest:
    task=unreal.AssetImportTask();task.filename=str(root/(name+'.wav'));task.destination_path='/Game/Audio'
    task.automated=True;task.replace_existing=True;task.save=True;tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
for name,info in manifest.items():
    sound=unreal.load_asset('/Game/Audio/'+name)
    assert sound,name
    sound.set_editor_property('looping',info['loop'])
    unreal.EditorAssetLibrary.save_loaded_asset(sound)
unreal.log('DUNGEON_AUDIO_IMPORT_COMPLETE '+str(len(tasks)))
