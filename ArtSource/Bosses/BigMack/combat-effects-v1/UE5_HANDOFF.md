# UE5 handoff — Big Mack combat effects

Read repository instructions and inspect the game's current rendering/VFX and status-effect systems first. This pack supplies source art, not implemented effects. Preserve the original PNGs and generate cleaned/sliced outputs in a separate folder.

## Prompt 1: Cleanup and import

Inspect all four sheets and README.md. Remove unwanted haze/fringes, correct alpha, isolate complete frames and normalize pivots/scales without clipping food, dust or stars. Use the project's pixel-art filtering/import conventions and existing renderer. Do not assume Paper2D or Niagara is already used. Build frame manifests with inspected bounds. Preview cleaned output against both dark and light backgrounds and inspect in the dungeon at actual gameplay size.

## Prompt 2: Burger splat and flying ingredients

Trigger the splat once at a burger projectile's actual collision. Destroy/disable the projectile so damage and the effect cannot fire twice. Use the existing damage rules and apply the intended stun/slow only on valid player hits; wall impacts may show the visual without applying player status. Emit a bounded number of lettuce and cheese particles with varied initial velocities, gentle gravity and tumble/flutter, then remove them after a short lifetime. Keep particles cosmetic and non-colliding with gameplay actors. Use pooling or the established equivalent if appropriate. Prevent unbounded residue accumulation. Tune count/lifetime to preserve readability; no new damage values are specified by this art task.

## Prompt 3: Landing shockwave

Trigger this physical dust/pressure burst once on confirmed Big Mack landing, not at takeoff and not as a warning. Align the effect to the floor beneath the boss. Play contact, expansion and fade in sequence and clean it up. Do not add ground-warning circles, target reticles or persistent markers. This request does not authorize new shockwave damage, knockback or screen shake: retain any existing gameplay behavior and keep this addition visual unless later requested otherwise.

## Prompt 4: Orbiting stun stars

Attach three gold stars above the player only while the actual stun status is active. Prefer smooth procedural elliptical orbit using delta time and three phase offsets 120 degrees apart; use cleaned individual star sprites. Render far stars behind and near stars ahead of the head where the existing renderer permits. Avoid a static full ring. Keep orbit center attached during movement/knockback and avoid animation-frame-dependent speed. Remove stars on stun expiration, player death, reset and room teardown. A remaining slow effect must not keep showing stun stars after the stun ends. Repeated hits should follow the game's status stacking policy and never create duplicate permanent star emitters. Status durations remain configurable; this art task does not choose them.

## Prompt 5: Validation

Verify burger hits on player and walls, one impact per projectile, simultaneous hits, stun expiration while slow persists, player death/restart while stunned, consecutive Big Mack landings and pause/resume. Inspect effects at several frame rates and with multiple particles active. Check lifetime cleanup and existing boss behavior. Capture short in-engine previews of each effect. Only mark integration complete after those checks; the current pack has not passed them.

