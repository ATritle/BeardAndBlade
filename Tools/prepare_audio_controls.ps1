param([Parameter(Mandatory=$true)][string]$Source)
Add-Type -AssemblyName System.Drawing
$root=(Resolve-Path "$PSScriptRoot/..").Path
Copy-Item -LiteralPath $Source -Destination "$root/ArtSource/AudioControlsAtlas.png"
$atlas=[System.Drawing.Bitmap]::new($Source)
$names=@('AudioMusic','AudioEffects','AudioRail','AudioThumb')
for($i=0;$i -lt 4;$i++) {
    $w=[int]($atlas.Width/2); $h=[int]($atlas.Height/2)
    $x=($i%2)*$w; $y=[int][Math]::Floor($i/2)*$h
    $left=$w; $top=$h; $right=0; $bottom=0
    for($dy=0;$dy -lt $h;$dy++){for($dx=0;$dx -lt $w;$dx++){
        if($atlas.GetPixel($x+$dx,$y+$dy).A -gt 128){
            $left=[Math]::Min($left,$dx);$right=[Math]::Max($right,$dx)
            $top=[Math]::Min($top,$dy);$bottom=[Math]::Max($bottom,$dy)
        }
    }}
    $rect=[System.Drawing.Rectangle]::new($x+$left,$y+$top,$right-$left+1,$bottom-$top+1)
    $crop=$atlas.Clone($rect,[System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $crop.Save("$root/Content/Art/V2/$($names[$i]).png",[System.Drawing.Imaging.ImageFormat]::Png)
    $crop.Dispose()
}
$atlas.Dispose()
