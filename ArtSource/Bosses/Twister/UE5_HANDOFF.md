# Twister — UE5 implementation handoff

These prompts describe remaining work, not completed or tested features. Read repository instructions and inspect the current implementation before making changes. Use the existing UE 5.8 project and established rendering/animation system; do not assume Paper2D is in use. Preserve other bosses and unrelated user changes.

## Accepted design

Twister is the red/orange tornado in reference.png, with angry white eyes, gloved hands and two M4-style rifles. His normal attack fires single bullets; his second attack fires full auto. Keep the tornado visibly swirling while aiming and attacking. Support all eight facing directions. The surrounding emblem is not part of the runtime sprite.

## Prompt 1 — Clean and prepare sprite frames

Work from ArtSource/Bosses/Twister/reference.png, README.md and directional-v1. Clean fringe pixels and inspect rifle, eye and silhouette consistency against the reference. Preserve two rifles and the red/orange funnel identity. Extract each of the 24 poses per direction using inspected bounds: sheets are 1448x1086 and are not suitable for naive equal integer grid cuts. Avoid clipping long barrels or flashes and mixing adjacent sprites. Align frames on a consistent canvas, scale and funnel-tip pivot. Correct directional drift and inconsistent rifle/hand shapes. Save cleaned art separately from the original source sheets and retain a reproducible frame manifest. Preview the 12-frame swirl loop and add or correct in-betweens to eliminate jumps. Do not claim smoothness from frame count alone. Consider separate muzzle-flash layers so flashes remain synchronized with gameplay.

## Prompt 2 — Import and animate in UE5

Import the cleaned frames using the project's existing texture/sprite workflow and pixel-art settings. Build eight-direction animations for swirl/idle-movement, single-shot and automatic fire. Source frame indices: 0–11 swirl, 12–17 single-shot, 18–23 automatic fire, row-major. Treat these as source pose intent, not rigid event timings. Keep eyes and rifles aimed while wind ribbons move. Use stable pivots and smooth directional selection without jitter. Start swirl previews around 12–16 fps, then tune visually. Keep movement and continuous swirl active during attacks where appropriate. Add muzzle origins per rifle/direction; do not attach bullets to an arbitrary sprite center. Use the current renderer rather than introducing a new animation framework unnecessarily. Verify front, back, side and diagonal views in the actual dungeon.

## Prompt 3 — Implement boss AI and attacks

Integrate Twister with the existing boss lifecycle, damage, health bar, room gating, dialogue and reward systems. Inspect the current roster before choosing his encounter slot; do not silently replace Finance Guy or another boss. If no slot is specified, make Twister spawnable in a test encounter and leave final campaign placement pending. Normal attack: telegraphed aimed single projectiles from one rifle at a time, with matching flash/recoil and a recovery interval. Secondary attack: clearly telegraphed, finite full-auto burst using the two rifles, followed by a readable cooldown. Suggested baseline is alternating rifles, not an unrequested hitscan attack. Keep bullet damage, speed, range/lifetime, normal-shot cadence, burst rate/duration, recovery, health and movement speed configurable and document chosen provisional defaults. Spawn bullets on actual shot events independent of rendering frame rate. Use the existing collision and damage rules; prevent bullets from damaging Twister, duplicating hit damage or passing through walls unintentionally. Respect pause/dialogue/death states. Stop attack timers and effects on death, room teardown and restart. Use existing audio only when appropriate and mark temporary audio clearly; no new recorded audio has been supplied.

## Prompt 4 — Verify and package

Run appropriate existing campaign and packaged smoke checks, plus focused Twister checks: all eight facings; stable 12-frame swirl loop; one bullet per normal-shot event; bounded automatic burst; matching muzzle origins/flashes; frame-rate-independent fire cadence; projectile collision/lifetime; pause and dialogue suppression; player death/restart; boss death mid-burst; room completion and one reward. Inspect gameplay visually and listen to attack audio. Confirm other bosses still work. Build/cook/package a fresh Windows build only after import and gameplay validation. Report what passed and any unresolved art or balance issues. Do not describe these source sheets as production-ready until cleanup and in-engine playback have been verified.

## Decisions still open

- Campaign boss slot and dialogue.
- Health, damage, projectile speed, attack cadence, burst length and cooldowns.
- Movement pattern and difficulty balance.
- Final voice lines, rifle sounds and tornado audio.

No stun/slow effect is specified for Twister; those effects belong to Big Mack's burger attack.
