param([string]$Engine='C:/Program Files/Epic Games/UE_5.8',[string]$Destination='')
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if(!$Destination) { $Destination=Join-Path $projectRoot 'Builds/v0.2.0' }
if(Test-Path (Join-Path $Destination 'Windows')) { throw 'Choose a fresh archive destination to avoid shipping leftover files from older builds.' }
$env:uebp_EngineSavedFolder=Join-Path $projectRoot 'Saved/Automation'
& "$Engine/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun "-project=$projectRoot/TheBeardAndBlade.uproject" -noP4 -platform=Win64 -clientconfig=Shipping -build -cook -stage -pak -iostore -archive "-archivedirectory=$Destination" -prereqs -nodebuginfo -utf8output -unattended
if($LASTEXITCODE -ne 0) { throw "Unreal packaging failed: $LASTEXITCODE" }
$brandOutput=Join-Path $Destination 'Windows/Branding'
New-Item -ItemType Directory -Force $brandOutput | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'Branding/Icon.png'),(Join-Path $projectRoot 'Branding/Logo.png'),(Join-Path $projectRoot 'Branding/PROMPTS.md') -Destination $brandOutput
Write-Output "Package ready in $Destination. Distribute the whole Windows folder, not only the EXE."
