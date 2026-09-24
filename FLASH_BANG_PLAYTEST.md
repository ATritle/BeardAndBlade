# Flash Bang Guy — local UE5 playtest

Open `TheBeardAndBlade.uproject`, select Play in Selected Viewport, then press **F11** with the gameplay viewport focused to jump to room 9. This shortcut resets the run and gives test equipment. F6/F10 select Twister, now room 21. See [current progression](PROGRESSION_PLAYTEST.md) for every chapter and shortcut.

## Encounter

- Rooms 7–9: Blackout Bunker, with concrete, military crates, radios, sandbags and three built-in exits. Room 9 is Flash Bang Guy. Rooms 19–21 are Stormbreach Citadel for Twister.
- PMC soldier carries a slung rifle. Eight walking poses and eight grenade-throw/recovery poses; mirrored for left-facing movement. No rifle shots in this encounter.
- Grenade release follows the overhand throw. Flight lasts 0.8 seconds, followed by a visible ground fuse; detonation is 1.45 seconds after release. Grenades do not detonate on passing over the player.
- At detonation the game checks the player's actual aim direction, a 120-degree forward cone and a 300-unit radius. Turn the mouse aim away or leave that radius to avoid the flash effects.
- Exposure causes a 0.6-second stun, 2.5-second slow, 2.2-second fading screen blur, and 20 base damage before armor. It cannot repeatedly extend an existing flash. Freedom immunity remains respected.
- A successful stun from this boss's own grenade triggers a half-second teleport ambush: he appears beside the player, stabs once at 0.18 seconds for 25% of current health (after grenade damage, ignoring armor), and returns to his exact departure position. Avoided flashes and ordinary flash-trooper grenades do not trigger the ambush. A defeated grenade owner cannot ambush.
- The explosion is a world-space white burst. The exposed-player overlay is one fading white flash, not a strobe. A real Slate background blur affects the sprite-rendered game, not just the unused world camera. The flash report is original synthesized audio, not a recorded grenade.
- Pause/inventory hide the overlay and pause its countdown. Death, restart, room change and ending PIE clear it.

## Validation

Completed locally: Editor build succeeded; FlashVerify, DungeonVerify and SeptemberVerify each reported `errors=0`. The facing and looking-away screenshots were visually inspected: only the exposed case has the full-screen white fade and blur. This is an automated/render-reviewed build ready for your hands-on balance test, not a new packaged release.

- UE 5.8 Development Editor build.
- `-FlashVerify`: eight-direction cone/range checks, real hero avoidance/exposure, fuse, no early collision, one detonation, expiration/reset, frame-rate-independent throw, asset loading, room 9/21 mapping.
- `-DungeonVerify`: complete 21-room campaign including seven boss introductions, rewards and gate transitions.
- `-SeptemberVerify`: existing chest, inventory-full, Twister ten-round burst, damage and boss-order regression checks.
- `Tools/capture_review.ps1 -Preview Boss -Biome 2`: bunker/boss introduction screenshot (zero-based chapter index).
- `Tools/capture_review.ps1 -Preview Flash` and `-Left`: facing/looking-away rendered flash comparison. Review-only flags never run during normal gameplay.

Artwork was generated using the built-in image-generation tool from the supplied costume reference, then alpha-isolated, cropped and registered to a fixed foot pivot. Sources and prompt set: `ArtSource/FlashBang`; import script: `Tools/import_flashbang.py`. No GitHub push or packaged ZIP was made for this update.
