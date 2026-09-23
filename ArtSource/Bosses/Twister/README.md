# Twister — eight-direction animation source artwork

Built with the built-in image-generation tool using the supplied reference.png as the character identity reference. The circular emblem was omitted for standalone game sprites. These are pixel-art adaptations, not pixel-exact reproductions of the reference.

## Contents

Eight directional PNGs, each visually arranged in six columns and four rows: 24 poses per direction, 192 total. Directions use screen space: north away/up, south toward viewer/down, east right, west left, plus four diagonals. Actual images are 1448x1086 RGBA with transparent corner pixels, not the requested 2048x1536.

## Frame map

Read each row left to right, then top to bottom. Zero-based indices:
- 0–11 (rows 1–2): 12 evolving wind/swirl poses, intended as a continuous cycle.
- 12–17 (row 3): aim, anticipation, single-shot muzzle flash and recoil, recovery, settle, ready.
- 18–23 (row 4): brace, first-rifle burst, second-rifle burst, dual flash, recoil recovery, braced aim.

The wind ribbons should rotate while facing, eyes and weapons remain aimed consistently. The southeast sheet was regenerated to fix an initial reversed second row; only the corrected sheet is included.

## Required cleanup before Unreal integration

This is source art, not validated flipbook animation. Inspect and remove fringe pixels; preserve orange/yellow wind tips and muzzle flash edges. Check each frame's rifle anatomy and occlusion; dual M4-style rifles are the design requirement, not mechanically precise weapon illustrations. Some side/front views overlap the rifles heavily. Normalize eye design, silhouette, gun scale, frame scale and pivot across directions. Some muzzle flashes approach neighboring cells, so extract inspected individual bounds rather than automatic grid cuts. Width and height do not divide cleanly into the requested integer cell dimensions.

Preview the 12 swirl frames as a loop, correct silhouette jumps and add in-betweens where required. Keep a stable pivot at the funnel ground tip. Suggested starting playback: 12–16 fps for swirl; single-shot recoil over roughly 0.4 seconds; full-auto poses loop while actual bullet emission is controlled separately by gameplay. These are tuning suggestions, not tested values. Consider extracting muzzle flashes as separate effect sprites to prevent flicker and synchronize actual shots.

## Gameplay brief

Boss name: Twister. Red/orange swirling tornado with angry white eyes and white gloves. Wields two M4 rifles. Normal attack fires single bullets; secondary attack is full-auto fire. No damage, range, fire rate, burst duration, health or cooldown values have been specified. Art generation does not implement bullets, AI or collision.

No UE asset imports, gameplay changes or animation playback tests have been performed for this set. Original reference and full generation prompts are retained for later cleanup. See UE5_HANDOFF.md for the implementation prompts. Repository destination: ArtSource/Bosses/Twister.
