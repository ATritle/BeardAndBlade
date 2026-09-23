using UnrealBuildTool;
public class TheBeardAndBladeTarget : TargetRules { public TheBeardAndBladeTarget(TargetInfo Target) : base(Target) { Type = TargetType.Game; DefaultBuildSettings = BuildSettingsVersion.V7; IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8; ExtraModuleNames.Add("TheBeardAndBlade"); } }
