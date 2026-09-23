using UnrealBuildTool;
public class TheBeardAndBladeEditorTarget : TargetRules { public TheBeardAndBladeEditorTarget(TargetInfo Target) : base(Target) { Type = TargetType.Editor; DefaultBuildSettings = BuildSettingsVersion.V7; IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8; ExtraModuleNames.Add("TheBeardAndBlade"); } }
