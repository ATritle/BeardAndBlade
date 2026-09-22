import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
exec((root/'Tools/import_art_v2.py').read_text())
root=Path(unreal.Paths.project_dir())
# Reimport normalized campaign frames when the slicing metadata changes.
import json
tasks=[]
for name in json.loads((root/'ArtSource/campaign_frame_manifest.json').read_text()):
    task=unreal.AssetImportTask();task.filename=str(root/'Content/Art/V2'/(name+'.png'))
    task.destination_path='/Game/Art/V2';task.automated=True;task.replace_existing=True;task.save=True
    tasks.append(task)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
path='/Game/Art/V2/M_KeySprite'
material=unreal.load_asset(path)
if not material:
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_KeySprite','/Game/Art/V2',unreal.Material,unreal.MaterialFactoryNew())
    lib=unreal.MaterialEditingLibrary
    material.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('two_sided',True)
    tex=lib.create_material_expression(material,unreal.MaterialExpressionTextureSampleParameter2D,-600,0)
    tex.set_editor_property('parameter_name','SpriteTexture')
    tex.set_editor_property('texture',unreal.load_asset('/Game/Art/V2/Creature_0_0'))
    tint=lib.create_material_expression(material,unreal.MaterialExpressionVectorParameter,-600,250)
    tint.set_editor_property('parameter_name','Tint');tint.set_editor_property('default_value',unreal.LinearColor(1,1,1,1))
    key=lib.create_material_expression(material,unreal.MaterialExpressionCustom,-300,160)
    key.set_editor_property('code','return 1.0-smoothstep(0.25,0.42,min(C.r,C.b)-C.g);')
    key.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    inp=unreal.CustomInput();inp.set_editor_property('input_name','C');key.set_editor_property('inputs',[inp])
    lib.connect_material_expressions(tex,'RGB',key,'C')
    alpha=lib.create_material_expression(material,unreal.MaterialExpressionMultiply,-50,160)
    lib.connect_material_expressions(key,'',alpha,'A');lib.connect_material_expressions(tint,'A',alpha,'B')
    lib.connect_material_property(alpha,'',unreal.MaterialProperty.MP_OPACITY)
    gamma=lib.create_material_expression(material,unreal.MaterialExpressionPower,-300,-100)
    gamma.set_editor_property('const_exponent',1.0/2.2);lib.connect_material_expressions(tex,'RGB',gamma,'Base')
    color=lib.create_material_expression(material,unreal.MaterialExpressionMultiply,-50,-100)
    lib.connect_material_expressions(gamma,'',color,'A');lib.connect_material_expressions(tint,'RGB',color,'B')
    lib.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.recompile_material(material);unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log('CAMPAIGN_ART_IMPORT_COMPLETE')
