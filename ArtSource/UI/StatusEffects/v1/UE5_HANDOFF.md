# Status effect source pack v1

Created with built-in image_gen; exact prompt is in generation-prompts.json.

status-effects-8-frame-atlas.png is 1586 x 992 RGBA. It contains five illustrated rows, eight frames each, ordered left to right:

1. Stun: orbiting gold stars.
2. Slow: blue snail with rotating clock hand.
3. Poison: green skull droplet with rising bubbles.
4. Bleed: red slashed droplet with falling drop.
5. Temporary immunity: silver/cyan shield with traveling sparkle.

## Cleanup and integration prompt

The generator returned 1586 x 992 rather than the requested 1536 x 960. DO NOT slice this sheet as exact 192 x 192 cells. Manually isolate 40 frames, remove any colored alpha fringe, and normalize them onto equal transparent canvases with identical centers and padding. The falling bleed drop approaches the shield row: keep it attached only to its bleed frame. Keep the original atlas unchanged.

Inspect transparency over light and dark backgrounds. Corner alpha is zero, but that does not establish that every edge is clean. Inspect individual frames for stable silhouette and scale. Repair the loop seam and orbital timing; the star positions are not a mathematically even orbit. For a perfectly smooth stun animation, extract stars and animate them procedurally around an ellipse instead.

Start with 8 frames at 8 fps (one-second loop), then tune after a playback test. These are animation source sequences, not verified smooth runtime animations. Build a UMG animated icon widget using frame textures or a normalized atlas UV material. A Paper2D flipbook does not automatically animate a UMG Image.

Bind visibility to authoritative active effect state, not to the visual animation. Clear icons immediately when an effect ends, the actor dies, or the level resets. Show simultaneous effects in stable slots without overlap. Display duration separately from loop playback; the clock hand on Slow is decorative and must not imply a precise remaining time. Decide stacking and refresh rules from the actual gameplay effect implementation.

Keep stun and slow separate even when Big Mack applies both. Poison and bleed have distinct silhouettes as well as colors. Temporary immunity displays only while damage immunity is actually active. Provide text labels/tooltips for accessibility. Avoid rapid flashes; use gentle pulses. Check readability at 24, 32, and 48 screen pixels and reduce detail if necessary.

This package adds artwork only; it neither implements new gameplay effects nor changes damage/status logic. Test application, refresh, expiration, removal, simultaneous effects, pause/resume, and HUD scaling in UE at home.
