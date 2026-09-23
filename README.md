# The Beard and Blade

A Windows pixel-art dungeon crawler built with Unreal Engine 5.8.

## Play

Download `TheBeardAndBlade-Windows-v0.2.1.zip` from [Releases](https://github.com/ATritle/TheBeardAndBlade/releases). Extract the **entire ZIP**. Newly packaged ZIPs launch from `TheBeardAndBlade/TheBeardAndBlade.exe`. The existing v0.2.1 download uses `Windows/TheBeardAndBlade.exe`; rename its outer `Windows` folder to `TheBeardAndBlade` to use the new path. Unreal Editor is not required. The automatic GitHub source ZIP is not playable.

Windows x64 with a compatible DirectX graphics driver is required. Keep the supporting folders beside the EXE. If prerequisites are missing, run the bundled installer under `Engine/Extras/Redist/en-us` inside the extracted game folder. This is an unsigned playtest, not a browser, macOS or Linux build.

## Controls

WASD move; Shift sprint; Space dodge; mouse aim; LMB attack; RMB tea splash; MMB FREEDOM after 15 enemy kills; E interact; I inventory; P menu; M music; N effects.

Hover over inventory items for stats. Drag to rearrange the bag or drop on the matching equipment slot. Double-click to equip. Items cannot overlap. Unequip returns gear to an available bag space.

## Features

- Four dungeon themes, 24 regular enemy species and four bosses, appearing every fourth room.
- Multiple boss attacks, illustrated projectiles, blood effects and fading floor pools.
- Three random reward chests per cleared room: choose one, equip your loot, then enter a glowing background arch.
- 48 item designs with five rarity tiers, rolled stats and timed combat effects.
- Six-by-six inventory: weapons 1×2, armor 2×2 and amulets 1×1.
- Directional hero animations, hand-anchored equipment, individually sized weapons and a consistent starting outfit.
- Compact health/stamina HUD, potions, music, sound effects and an illustrated title menu.

## Develop

Open `TheBeardAndBlade.uproject` in Unreal Engine 5.8 with the C++ toolchain installed. Choose Play > Selected Viewport. Imported runtime assets are included; raw/private reference artwork, caches and packaged binaries are excluded from source control.

See [PLAYTEST.md](PLAYTEST.md) for testing, [DISTRIBUTION.md](DISTRIBUTION.md) for packaging, [AUDIO.md](AUDIO.md) for sound and [RELEASE_NOTES.md](RELEASE_NOTES.md) for this version.

## Playtest limitations

No saved campaign progress or ending; themes repeat after room 16. Armor changes stats without changing the hero's outfit colors. FREEDOM uses an eagle screech and graphical callout, not a recorded spoken voice. Balance and subjective animation/audio quality still need player feedback.
