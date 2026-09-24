# UE5 playtest: grips, Twister and boss-matched dungeons

Open TheBeardAndBlade.uproject in UE 5.8.2 and use Play / Selected Viewport. This update does not replace the previous packaged ZIP or publish to GitHub.

## Direct encounter testing (UE Editor builds only)

Click the game viewport to focus input. F1 Finance Guy (room 4), F2 Big Mack (8), F3 Webroot (12), F4 Rime (16), F5 Cinder (20), F6 Twister (24). F7 opens the first ice room (13). F9/F10 remain Finance/Twister aliases. F8 is left to Unreal's normal eject command. Each shortcut **resets the current run and inventory** and equips test weapon/armor. Boss dialogue still precedes combat. These shortcuts are not bound in packaged builds.

## Ember Drake fire attachment

Original Creature_19_5/6 have flames painted over the torso. Runtime now uses the existing clean open-jaw frame 4 during windup/emission instead; no source art was overwritten. A separate brief fire plume tracks the measured jaw socket, mirrored with left/right facing and following the same flying bob/breathing transform. Three fire projectiles launch forward from that mouth, not the floor/body center, and use corresponding chest-height player collision. Aim is restricted to the forward hemisphere so the plume cannot point backward through the dragon. The existing flying animation and attack cadence remain.

Tools/capture_review.ps1 -Preview Drake renders both mirrored mouth connections.

Validation: UE Editor module compiled; focused verification and 24-room campaign regression both returned zero errors. All six direct boss-start paths were verified. Both mirrored dragon mouth attachments were rendered and inspected in ArtSource/SeptemberQA/DrakeMouthReview.png; projectile origin/direction and clean windup/emission frames have regression assertions. Existing source sprites retain their low-resolution edges; this change corrects attachment/render selection, not a wholesale dragon redraw.

## Progression follow-up

Reward collection now works from the original chest interaction position as well as beside the dropped item. A persistent on-screen reminder explains pickup and, after a failed pickup, how to clear bag space. Inventory insertion still precedes healing and door unlocking; no loot is silently discarded. Existing full-bag cards only appeared after an in-range attempt, making the old too-small pickup zone particularly confusing.

Wave scheduling clears stale reward state. Late enemy spawns are blocked during rewards/transitions, rewards require zero live and queued enemies, and transitions require collected loot with no combat remaining. Freedom's active sweep is cleared on room change. ROOM_FLOW log entries identify scheduling, reward readiness and successful pickup. Finance overlap was not independently reproduced from the report; these guards enforce separation rather than claiming a confirmed root cause.

Regression results: focused checks and the 24-room campaign each pass with zero errors. New tests use actual spawn timers through room 3's chest, the gate transition and Finance's spawn/defeat. Room 13 is tested with a full 36-slot bag, a failed pickup, freeing a valid slot, successful pickup and passage into room 14. The direct Twister shortcut's reset path is also checked. Full-bag behavior intentionally does not unlock doors before collection.

- Individually measured handle pivots for all 24 catalog weapons; especially the off-center scythe, cleaver and curved blades. Existing per-animation hand sockets and finger occlusion remain.
- Twister bullets: 2200 screen units/sec (previously 500), re-aimed every shot with up to 0.22 seconds of movement prediction; no homing after launch. Existing windup and ten-shot burst/recovery remain. Swept collision checks protect against tunneling at low frame rates.
- Each bullet collision deals its own armor-adjusted damage, ignoring ordinary hurt grace. Dodge's first 0.34 seconds and Freedom still block bullets. Each projectile is removed after impact.
- Balance follow-up: Twister bullet base damage reduced 30%, from 24 to 16.8 before armor. Ten-shot bursts, speed, targeting, cooldowns and health are unchanged.
- Bullet artwork is a dedicated layered brass/white nose and short exposure trail, replacing the reused tea effect. Three original synthesized rifle cues combine muzzle crack, pressure transient, bolt and room reflections; these are not recorded firearm samples.
- Six encounter groups use themes Keep, Keep, Webroot, Winter, Fire, Storm. Boss rooms: Finance 4, Mack 8, Webroot 12, Rime 16, Cinder 20, Twister 24. The final theme draws from the existing normal enemy roster without accidentally spawning bosses as regular enemies.

## New artwork

Built-in image generation produced Content/Art/V2/Arena4.png, imported as Arena4.uasset. Reference: Arena1.png for framing and pixel-art style. Prompt: create Stormbreach Citadel for the tornado/rifle boss; dark slate-blue wind-scoured tiles, copper lightning rods, wind-torn ironwork at the border, amber and pale cyan lights; preserve three upper doorways and open rectangular floor; no characters, HUD, words or central tornado. Source generation retained in the local generated-images folder. Door glows continue to illuminate existing background arches.

## Review tools

Idle breathing update: stationary heroes blend into a roughly 1.37-second recovery-breath cycle, accelerating toward 0.92 seconds at empty stamina. Chest expansion, shoulder/head lift and a small forward lean deform the existing pose continuously in all eight directions; feet below the calves stay fixed. Weapon grip, finger occlusion and charm use the same mapping. The animation pauses with menus/inventory and is suppressed for attacks, casting, rolling and death. No breathing audio was added. Tools/capture_review.ps1 -Preview Idle shows inhale/exhale comparisons across all directions.

Editor module rebuilt successfully. Focused September verification and 24-room campaign verification report zero errors. All 24 equipped weapon types were rendered and inspected facing right and left; the off-center scythe now connects through its handle rather than empty space.

The rendered UE5 six-boss/reward smoke test also completed with zero errors. New environment and audio imports completed with zero errors. Rime, Cinder and Twister rendered against their winter, fire and storm backgrounds. Audio subjective realism and difficulty still require your listening/playtest feedback.

Tools/capture_review.ps1 -Preview Boss -Biome 5 previews Twister; -Biome 3 previews Rime and -Biome 4 Cinder. -Preview Weapons renders all catalog weapons in equipped poses. Automated -SeptemberVerify checks include consecutive projectile hits, low-frame-rate swept collisions and dodge immunity. -DungeonVerify checks the 24-room boss/theme progression.
