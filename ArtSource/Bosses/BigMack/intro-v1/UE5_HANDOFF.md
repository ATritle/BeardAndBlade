# Big Mack cinematic intro — v1

Matches the approved Twister intro-v5 structure, timing and synthesized musical sketch, with Big Mack artwork and burger-themed effects. Open preview.html with all adjacent files present, then Play cinematic. The backdrop is an illustrative floor; production must overlay the actual destination dungeon after portal arrival. Approved for repository upload. UE gameplay integration and runtime testing remain pending.

Separate transparent assets: big-mack-character.png and big-mack-title.png. Generation references are the earlier Big Mack boss concept and approved compact BossUI/v2 title style. Inspect alpha edges and compare character with the current runtime boss before production import.

Timeline: 0–0.30s dim room; 0.30–1.50s rush in from left and overshoot; settle by 1.68s; rest 0.42s; title starts at 2.10s and hits at 2.40s; hold until 5.10s; fade out by 6s. Character and name are independently animated. Music impact, flash and minor shake align to title hit. Reduced motion disables travel/effects and exposes the title with a fade.

effects.js provides pixel dust, tumbling lettuce/cheese/burger crumbs, entrance trails, a landing burst at 1.50s and smaller title burst at 2.40s. These are procedural browser effects, not supplied sprite sheets or UE particles. Recreate as appropriate UMG layers/particles or Niagara composited into UI. Keep particles away from eyes and below the nameplate. No ground warning circles.

The preview audio is a browser-synthesized arrangement, not a production WAV. Rebuild/export and mix an audio cue, respecting game volume. Use the same reusable intro controller as Twister: await room/camera readiness, gate player movement/attack and enemy AI, coordinate dialogue, trigger once, clear held inputs, support skip, and restore control exactly once. Cancel pending particles/audio on restart/death/room teardown. Validate frame rate independence, replay, safe areas, aspect ratios, transparency and gameplay transitions before packaging.
