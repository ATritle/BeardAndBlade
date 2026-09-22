using UnrealBuildTool;
public class BeardAndBladeEditorTarget : TargetRules { public BeardAndBladeEditorTarget(TargetInfo Target) : base(Target) { Type = TargetType.Editor; DefaultBuildSettings = BuildSettingsVersion.V7; IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8; ExtraModuleNames.Add("BeardAndBlade"); } }
