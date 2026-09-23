# Big Mack — eight-direction animation source sheets

Generated with built-in image_gen. Eight sheets, 16 poses each (128 poses). These are source animation studies, not validated runtime flipbooks.

## Directions

Directions use screen space: south toward viewer, north away, east screen-right, west screen-left; diagonals are northeast, northwest, southeast, southwest. All eight have separate PNGs.

## Frame map (row-major, zero-based)

- 0–3: idle/breathing: neutral, inhale, full inhale, exhale.
- 4–7: hop anticipation/compression, deepest squash, takeoff, rise.
- 8–11: apex, descent, landing squash, recovery.
- 12–15: burger held ready, windup, release/follow-through, recovery.

## Import preparation and QA

Actual generated dimensions are 1254x1254, RGBA, with transparent corner pixels, despite a requested 2048x2048 grid. Do NOT assume 512-pixel cells or automatically divide into equal integer cells. Extract each pose by inspected bounds; give every frame consistent canvas dimensions and pivot. Some poses approach cell boundaries. Inspect/clean edge fringes, frame scale and topping consistency. Southwest throw poses drift in facing and need correction. Normalize directional silhouettes and throwing-hand continuity before use. More in-between throw frames may be needed after preview. No smooth playback or engine import has yet been verified.

Suggested initial preview timing (not tested): idle 6 frames/sec with ping-pong playback; jump 12 frames/sec with a short apex hold; throw 10 frames/sec with projectile spawn at release. Tune to gameplay. Avoid applying baked vertical pose displacement plus a second jump displacement unintentionally.

Boss design: oversized double cheeseburger with sesame bun, lettuce, tomato and cheese; hops/jumps, throws burgers. Burger hits stun and slow the player. Durations and balance remain unspecified. No gameplay code or UE assets changed. Source artwork is stored here for later cleanup and UE5 integration. Pull main on the home PC to retrieve this folder.


