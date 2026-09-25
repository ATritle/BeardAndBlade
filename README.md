# The Beard and Blade

A Windows pixel-art dungeon crawler built with Unreal Engine 5.8.

## Play

Download `TheBeardAndBlade-Windows-v0.3.1.zip` from [Releases](https://github.com/ATritle/TheBeardAndBlade/releases). Extract the **entire ZIP** and launch `TheBeardAndBlade/TheBeardAndBlade.exe`. Unreal Editor is not required. The automatic GitHub source ZIP is not playable.

Windows x64 with a compatible DirectX graphics driver is required. Keep the supporting folders beside the EXE. If prerequisites are missing, run the bundled installer under `Engine/Extras/Redist/en-us` inside the extracted game folder. This is an unsigned playtest, not a browser, macOS or Linux build.

## Controls

WASD move; Shift sprint; Space dodge; mouse aim; LMB attack; RMB tea splash; MMB FREEDOM after 15 enemy kills; E interact; I inventory; P menu; M music; N effects.

Hover over inventory items for stats. Drag to rearrange the bag or drop on the matching equipment slot. Double-click to equip. Items cannot overlap. Unequip returns gear to an available bag space.

## Features

- Seven dungeon themes and seven bosses, appearing every third room. Finance Guy (3), Big Mack (6), Flash Bang Guy (9), Webroot (12), Rime (15), Cinder (18), Twister (21). The new Greaseworks, bunker and storm rosters add 18 animated enemies. See [current local UE5 playtest](PROGRESSION_PLAYTEST.md).
- Multiple boss attacks, illustrated projectiles, blood effects and fading floor pools.
- Three random reward chests per cleared room: choose one, watch it open and eject loot, then press E near the landed item to collect it. Full bags preserve the same floor item. Equip it from inventory, then enter a glowing background arch.
- 48 item designs with five rarity tiers, rolled stats and timed combat effects.
- Six-by-six inventory: weapons 1×2, armor 2×2 and amulets 1×1.
- Directional hero animations, hand-anchored equipment, individually sized weapons and a consistent starting outfit.
- Compact health/stamina HUD, potions, music, sound effects and an illustrated title menu.
- Seven illustrated boss entrance cinematics with skip, pause, reduced-motion (R), themed effects and synchronized audio.
- Illustrated death/restart screen and campaign victory with a closing story after Twister in room 21. No final chest or room 22.

## Develop

Open `TheBeardAndBlade.uproject` in Unreal Engine 5.8 with the C++ toolchain installed. Choose Play > Selected Viewport. Imported runtime assets are included; raw/private reference artwork, caches and packaged binaries are excluded from source control.

See [PLAYTEST.md](PLAYTEST.md) for testing, [DISTRIBUTION.md](DISTRIBUTION.md) for packaging, [AUDIO.md](AUDIO.md) for sound and [RELEASE_NOTES.md](RELEASE_NOTES.md) for this version.

## Playtest limitations

No saved campaign progress. The campaign ends after room 21; New Run starts fresh. Armor changes stats without changing the hero's outfit colors. FREEDOM uses an eagle screech and graphical callout, not a recorded spoken voice. Balance and subjective animation/audio quality still need player feedback. See [current progression playtest](PROGRESSION_PLAYTEST.md) for the latest room order and editor shortcuts; older integration notes describe earlier builds. Browser hosting requires a separately prepared Pixel Streaming build; this Windows release does not enable that plugin.
