import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
names=[('BigMack/intro-v1/big-mack-character.png','IntroMackCharacter'),('BigMack/intro-v1/big-mack-title.png','IntroMackTitle'),('Twister/intro-v5/twister-character.png','IntroTwisterCharacter'),('Twister/intro-v5/twister-title.png','IntroTwisterTitle')]
for boss in ('Finance','Flash','Webroot','Rime','Cinder'):
    for layer in ('Character','Title'):
        names.append((f'{boss}/intro-v1/{layer.lower()}.png',f'Intro{boss}{layer}'))
for file,name in names+[('../../Content/Audio/BossIntroCue.wav','BossIntroCue')]:
    audio=name=='BossIntroCue';path=root/'Content/Audio/BossIntroCue.wav' if audio else root/'ArtSource/Bosses'/file
    task=unreal.AssetImportTask();task.filename=str(path);task.destination_path='/Game/Audio' if audio else '/Game/Art/Intros';task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);assert task.imported_object_paths,path
    asset=unreal.load_asset(task.imported_object_paths[0])
    if audio:asset.set_editor_property('looping',False)
    else:
        asset.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
        asset.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        asset.set_editor_property('never_stream',True)
        asset.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log(f'BOSS_INTROS_IMPORTED {len(names)} alpha layers + 1 synchronized cue')
