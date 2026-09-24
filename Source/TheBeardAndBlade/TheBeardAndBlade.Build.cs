using UnrealBuildTool;
public class TheBeardAndBlade : ModuleRules { public TheBeardAndBlade(ReadOnlyTargetRules Target) : base(Target) { PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs; PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" }); PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" }); } }
