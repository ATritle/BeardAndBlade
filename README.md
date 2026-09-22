# Beard & Blade

Playable Unreal Engine 5.8 sprite dungeon game. See PLAYTEST.md for controls, content, validation and current limitations.

## Play

**Players:** download the Windows ZIP from [Releases](https://github.com/ATritle/BeardAndBlade/releases), extract the entire folder, and launch `BeardAndBlade.exe`. Unreal Editor is not required for the packaged build. The automatic GitHub source ZIP is not a playable game. See [DISTRIBUTION.md](DISTRIBUTION.md) for packaging, prerequisites and browser-play options.

**Developers:**

1. Open `BeardAndBlade.uproject` in Unreal Engine 5.8.
2. If prompted, allow Unreal to compile the C++ project files.
3. Choose **Play > Selected Viewport**, then **Begin Descent** on the title menu.

Controls: **WASD** move, **Shift** sprint, **Space** dodge, mouse aim, **left mouse** attack, **right mouse** tea splash (10-second cooldown), **E** interact, **I** inventory, **P** menu.

**M** toggles music; **N** toggles sound effects. Includes original synthesized menu, dungeon and boss music plus 16 gameplay effects. Details and regeneration instructions: [AUDIO.md](AUDIO.md).

The Tea & Finance update adds the bald, full-bearded, round-glasses adventurer, Finance Guy as the room-four boss, stock/bond volleys, and illustrated ranged projectiles and impact splashes. Artwork sources and built-in image-generation prompts are in `ArtSource/TEA_PROMPTS.md`.

## Implemented loop

Latest source update: compact bottom-center health/stamina HUD, stamina-limited sprint and dodge, health-potion drops, stronger enemies, small click-through boss speech bubbles, and three mystery chests that each roll from the full weapon/armor/amulet pool. Choose one reward; the others disappear. Includes Windows icon and splash branding. See [COMPACT_UPDATE.md](COMPACT_UPDATE.md) for details. Existing downloadable releases may predate these source changes.

Four dungeon themes with 24 regular species and four bosses. Bosses appear every fourth room. Clear a room, choose one of three chests, equip loot through the 6x6 inventory, then walk through a glowing gate. The four-theme cycle repeats after room 16 with increasing enemy health.

## Art handoff

Runtime sprites and dungeon/menu textures live in `Content/Art/V2`; audio lives in `Content/Audio`. Imported assets are included in Git. Raw art-generation sheets, private reference photographs and local render captures are retained only in the developer workspace, not uploaded. `Tools` contains the preparation/import scripts; art preparation requires those optional local source sheets, but opening/building the checked-in project does not. Rendering is a single Canvas layer with screen-space movement, frame-specific weapon anchors and worn armor variants. No external plugin is needed to play. Progress is not saved between sessions.
