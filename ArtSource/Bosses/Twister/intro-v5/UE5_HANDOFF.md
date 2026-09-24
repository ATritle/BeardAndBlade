# Twister intro v4: rush, settle, title

Supersedes the long 2.5-second pause in v3. Artwork is unchanged: separate transparent character and name graphics, with no baked dungeon background. Preview has a placeholder floor and synthesized music, not recorded gameplay.

Six-second sequence:
- 0.00–0.30: dim actual destination dungeon; intro starts after portal travel and room readiness.
- 0.30–0.90: Twister flies from fully offscreen LEFT, slightly tilted and scaled, to a 6% overshoot beyond his center position.
- 0.90–1.20: quick recoil and settle into final centered pose.
- 1.20–1.68: hold still for 0.48 seconds.
- 1.68–1.98: TWISTER title rushes in from left.
- 1.98–2.22: title impact, brief brightness hit, restrained flash and slight shake, scale settles.
- 2.22–5.10: hold composition.
- 5.10–6.00: fade overlay, restore encounter flow.

Music preview now hits at 1.98 seconds to match title arrival, with an earlier whoosh and smaller character arrival thump. It uses browser Web Audio synthesis, not a final WAV soundtrack. Preserve mute/volume; create/export production audio separately for UE.

Implement using separate UMG images and synchronized timeline/audio. Keep the actual newly entered dungeon visible underneath. Gate player movement/attacks and enemy AI; do not overlap boss dialogue. Trigger once, support skip and cancellation, restore controls once and clear held attack input. Honor reduced-motion/flash and audio settings. Inspect alpha edges, safe areas and timing in UE. No GitHub upload or Unreal code changes performed.

Motion correction: automatic prefers-reduced-motion CSS previously suppressed the boss slide in environments requesting reduced motion. Preview now has an explicit Reduced motion checkbox, off by default for this user-requested animation demonstration. Boss flight is slower and visibly traverses the screen: starts 0.30s, overshoots at 1.50s, settles at 1.68s, rests 0.42s, title starts at 2.10s and impacts at 2.40s. This supersedes timings above; music impact is resynchronized. Production game should respect accessibility preferences and expose an override. Browser visual verification was blocked by the file URL policy.

V5 effects: effects.js adds procedural pixel dust, shaded rotating stone fragments and low wind arcs on layers behind/in front of the boss but beneath the title. Tracks the moving boss bounds during entrance. Long rear wind streaks end after arrival; burst at 1.5s and smaller title burst at 2.4s. Effects fade out with overlay and are suppressed in Reduced motion. These effects are browser-rendered preview code, not sprite sheets or Unreal particles. Recreate in UMG/Niagara with equivalent depth ordering; preserve eye/rifle/name readability. Original 2.5-second title delay remains superseded by the short pause.
