param([string]$Preview='Menu',[int]$Biome=0,[switch]$Left)
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$taskRoot=(Resolve-Path (Join-Path $projectRoot '../..')).Path
$argsLine='"'+$projectRoot+'/BeardAndBlade.uproject" -game -windowed -ResX=1280 -ResY=800 -DungeonCapture -Dungeon'+$Preview+'Preview -DungeonBiome='+$Biome+' -nosound -unattended -NoSplash -DDC-ForceMemoryCache -LocalDataCachePath="'+$taskRoot+'/work/DDC" -ShaderWorkingDir="'+$taskRoot+'/work/Shaders" -abslog="'+$taskRoot+'/work/render-'+$Preview+$Biome+'.log"'
if($Preview -eq 'Equipment') { $argsLine=$argsLine.Replace('-DungeonEquipmentPreview','-DungeonEquipmentReview') }
if($Left) { $argsLine+=' -ReviewLeft' }
$reviewProcess=Start-Process -FilePath 'C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe' -ArgumentList $argsLine -WindowStyle Hidden -PassThru
$reviewProcess.WaitForExit()
Write-Output ($Preview+': exit '+$reviewProcess.ExitCode)
