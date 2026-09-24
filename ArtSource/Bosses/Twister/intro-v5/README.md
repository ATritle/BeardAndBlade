# Approved Twister intro v5 — authoritative integration instructions

This user-approved package is source artwork plus a browser demonstration, not implemented UE gameplay. These instructions supersede the earlier timings retained in UE5_HANDOFF.md.

Files: twister-character.png and twister-title.png are independent transparent layers. preview.html contains the animation and synthesized music arrangement; effects.js contains procedural pixel dust, rotating stones, wind arcs and bursts. Keep these files together to play the preview. generation-prompts.json records artwork provenance; historical source paths are not runtime dependencies.

## Final timeline

0–0.30s: dim the actual destination dungeon slightly.
0.30–1.50s: character flies from fully offscreen LEFT with tilt/scale, then overshoots center.
1.50–1.68s: recoil and settle.
1.68–2.10s: hold still for 0.42 seconds.
2.10–2.40s: name graphic rushes from left.
2.40–2.64s: name impact, brief restrained flash, small shake, scale settle.
2.64–5.10s: hold with swirling dust and debris.
5.10–6.00s: fade overlay and resume encounter flow.

## Home UE integration

Read project instructions and preserve local changes before pulling. Trigger once after portal travel into the boss room, once destination loading and camera placement are complete. Keep the ACTUAL dungeon behind the overlay; the browser floor is only illustrative. Build a reusable UMG intro with independent character/title layers and correct PNG alpha. Clean fringes and preserve safe margins/aspect ratio.

Recreate effects.js behavior as suitable UI sprite particles or Niagara composited into the overlay. Dust and stones pass behind and in front of the boss, below the title. Keep eyes, weapons and text readable. Wind trails follow entrance; arrival burst at 1.50s and smaller title burst at 2.40s. Fade and clear particles at end/cancel. Preview audio is Web Audio synthesis, not a supplied WAV; export/rebuild a production cue and mix its whoosh, pulses, riser and chord impact at 2.40s. No external music recording is included.

Gate player attacks/movement and enemy AI during intro; coordinate dialogue to avoid overlap. Clear held input and restore control exactly once after finish or skip. Respect reduced-motion/flash and audio settings. Handle pause, restart, death, re-entry and cancellation. Validate timing, multiple resolutions, low frame rate, input spam and light/dark backgrounds. Compile and test actual gameplay before packaging. These uploads do not update the official executable by themselves.

## Apply this format to Big Mack next

The user requests this SAME format for Big Mack: independent transparent boss and title, visible left entrance, overshoot/settle, 0.42-second rest, flashy title hit, intense synchronized music, and layered dust/debris. Use the actual Big Mack references; adapt debris to lettuce, cheese and burger crumbs. No baked background. Big Mack intro artwork and preview remain to be created/reviewed; do not treat Twister's files as Big Mack assets or overwrite existing Big Mack gameplay art.
