import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir())
exec((root/'Tools/import_art_v2.py').read_text())
path='/Game/Art/V2/M_WornArmor'
material=unreal.load_asset(path)
if not material:
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_WornArmor','/Game/Art/V2',unreal.Material,unreal.MaterialFactoryNew())
if material:
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    material.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('two_sided',True)
    lib=unreal.MaterialEditingLibrary
    color=lib.create_material_expression(material,unreal.MaterialExpressionTextureSampleParameter2D,-500,-100)
    color.set_editor_property('parameter_name','ArmorTexture')
    color.set_editor_property('texture',unreal.load_asset('/Game/Art/V2/Warden_WalkCardinal_2_0'))
    mask=lib.create_material_expression(material,unreal.MaterialExpressionTextureSampleParameter2D,-500,160)
    mask.set_editor_property('parameter_name','HeroMask')
    mask.set_editor_property('texture',unreal.load_asset('/Game/Art/V2/WalkCardinal_2_0'))
    tint=lib.create_material_expression(material,unreal.MaterialExpressionVectorParameter,-500,-320)
    tint.set_editor_property('parameter_name','Tint')
    tint.set_editor_property('default_value',unreal.LinearColor(1,1,1,1))
    mul=lib.create_material_expression(material,unreal.MaterialExpressionMultiply,-100,-100)
    lib.connect_material_expressions(color,'RGB',mul,'A');lib.connect_material_expressions(tint,'RGB',mul,'B')
    # Canvas material tiles do not apply the texture-tile sRGB display conversion.
    gamma=lib.create_material_expression(material,unreal.MaterialExpressionPower,100,-100)
    gamma.set_editor_property('const_exponent',1.0/2.2)
    lib.connect_material_expressions(mul,'',gamma,'Base')
    lib.connect_material_property(gamma,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    lib.connect_material_property(mask,'A',unreal.MaterialProperty.MP_OPACITY)
    lib.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
unreal.log('WORN_ARMOR_IMPORT_COMPLETE')
