# Dungeon progression — local UE5 test

Open `TheBeardAndBlade.uproject` in UE 5.8, choose **Selected Viewport** in the Play dropdown, then press Play. No packaged executable or GitHub release was made for this update.

## Campaign

Each chapter has two standard rooms (two staggered waves per room), followed by one boss room. Chest choice, inventory pickup and the existing doorway transition remain required before proceeding.

| Standard rooms | Boss room | Boss | Dungeon |
|---|---|---|---|
| 1–2 | 3 | Finance Guy | Forgotten Keep |
| 4–5 | 6 | Big Mack | The Greaseworks — new kitchen dungeon |
| 7–8 | 9 | Flash Bang Guy | Blackout Bunker |
| 10–11 | 12 | Webroot | Webroot Hollows |
| 13–14 | 15 | Rime | Glacial Reliquary |
| 16–17 | 18 | Cinder | Cinder Foundry |
| 19–20 | 21 | Twister | Stormbreach Citadel |

The existing endless loop continues after room 21; this update does not add an ending or save system.

## New enemies

All 18 have their own eight-frame sprite set: four movement poses and four attack poses, with left/right facing. Grounded movement has a subtle breathing/bob pass; flying enemies hover. Existing ailment flashes and health bars apply.

- **Greaseworks:** Patty Brute (heavy slam), Fry Skitter (charge), Pickle Lobber (arcing brine), Onion Bat (sonic fan), Cleaver Cook (melee cleave), Soda Imp (soda projectile).
- **Bunker:** Rifle Trooper (fast bullet), Shield Breacher (rush), Flash Cadet (facing-dependent flash grenade), Scout Drone (three-shot burst), Shock Trooper (electric close-range strike), Mortar Engineer (fused explosive).
- **Storm:** Tempest Wisp (wind blade), Thunder Roc (flying dive), Storm Knight (lightning sword), Static Spider (electric fan), Rubble Golem (heavy slam), Cyclone Imp (radial gust).

The roster cycles through all six species across each ordinary room's two waves, with a random starting species. Twelve new projectile/impact graphics accompany the themed attacks; rifle attacks reuse the existing rifle audio and bullet rendering.

## Test shortcuts (editor only)

Focus the game viewport first. Use the number row above QWERTY (not a numeric keypad). Shortcuts reset the run and supply level-appropriate test equipment. Existing function-key shortcuts also remain available.

| Key | Destination |
|---|---|
| 1 | Greaseworks standard enemies, room 4 |
| 2 | Bunker standard enemies, room 7 |
| 3 | Storm standard enemies, room 19 |
| 4 / F1 / F9 | Finance Guy, room 3 |
| 5 / F2 | Big Mack, room 6 |
| 6 / F11 | Flash Bang Guy, room 9 |
| 7 / F3 | Webroot, room 12 |
| 8 / F4 | Rime, room 15 |
| 9 / F5 | Cinder, room 18 |
| 0 / F6 / F10 | Twister, room 21 |
| F7 | First ice room, room 13 |

## Validation

- Development Editor build and 157 new texture imports.
- `-DungeonVerify`: 21 sequential rooms, all seven boss introductions, reward selection/pickup and gate transitions.
- `-SeptemberVerify`: boss attacks, Twister damage/bursts and reward/boss-spawn regressions.
- `-FlashVerify`: facing cone, grenade fuse, exposure, effect expiry and reset.
- `-ProgressionVerify`: all room/theme/roster mappings, 144 enemy frames, 12 effect textures, every new attack dispatch, projectile hit damage, fast-bullet swept collision and mortar fuse.
- Staged in-engine roster captures: `ArtSource/ThemeRosterReview6.png`, `ThemeRosterReview5.png`, `ThemeRosterReview4.png`.

These automated checks and staged captures are not a complete manual balance playthrough. Please test attack readability, difficulty and animation feel in the viewport before approving a release.

## Art sources

Built-in image generation supplied the new arena, three character atlases and effect atlas. Sources and exact prompts: `ArtSource/Progression/PROMPTS.md`. Normalized sprites and imported Unreal assets: `Content/Art/Progression`; arena: `Content/Art/V2/Arena6`. The preparation script records crop coordinates and uses one scale per species to avoid frame-size jitter.
