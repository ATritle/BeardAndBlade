param([string]$Engine='C:/Program Files/Epic Games/UE_5.8',[string]$Destination='')
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if(!$Destination) { $Destination=Join-Path $projectRoot 'Builds/v0.3.2' }
if(Test-Path (Join-Path $Destination 'Windows')) { throw 'Choose a fresh archive destination to avoid shipping leftover files from older builds.' }
$env:uebp_EngineSavedFolder=Join-Path $projectRoot 'Saved/Automation'
# Cook and UnrealPak must use the same Zen store, including child processes.
${env:UE-LocalDataCachePath}=Join-Path $projectRoot 'Intermediate/DDC'
# Build explicitly so UBA uses a writable project-local cache rather than ProgramData.
foreach($target in @(@('TheBeardAndBladeEditor','Development'),@('TheBeardAndBlade','Shipping'))) {
    & "$Engine/Engine/Binaries/ThirdParty/DotNet/10.0/win-x64/dotnet.exe" "$Engine/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.dll" $target[0] Win64 $target[1] "-Project=$projectRoot/TheBeardAndBlade.uproject" -WaitMutex -NoHotReloadFromIDE "-UBARootDir=$projectRoot/Intermediate/UBA"
    if($LASTEXITCODE -ne 0) { throw "Unreal compilation failed: $LASTEXITCODE" }
}
& "$Engine/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun "-project=$projectRoot/TheBeardAndBlade.uproject" -noP4 -platform=Win64 -clientconfig=Shipping -skipbuildeditor -nocompileeditor -nocompile -cook -stage -pak -iostore -archive "-archivedirectory=$Destination" "-AdditionalCookerOptions=-DDC=InstalledNoZenLocalFallback -DDC-ForceMemoryCache -LocalDataCachePath=$projectRoot/Intermediate/DDC -ShaderWorkingDir=$projectRoot/Intermediate/Shaders" -prereqs -nodebuginfo -utf8output -unattended
if($LASTEXITCODE -ne 0) { throw "Unreal packaging failed: $LASTEXITCODE" }
$brandOutput=Join-Path $Destination 'Windows/Branding'
New-Item -ItemType Directory -Force $brandOutput | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'Branding/Icon.png'),(Join-Path $projectRoot 'Branding/Logo.png'),(Join-Path $projectRoot 'Branding/PROMPTS.md') -Destination $brandOutput
Write-Output "Package ready in $Destination. Distribute the whole Windows folder, not only the EXE."
