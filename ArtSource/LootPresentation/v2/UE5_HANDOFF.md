# UE5 loot presentation implementation prompt

Use the existing chest asset and equipment catalog; this source pack extends their presentation. Inspect project instructions before modifying code. No UE build has been performed on the work PC.

## Prepare artwork

Required fix before integration: the interactive preview showed bottom clipping when it sampled equal grid cells. Do NOT reuse that preview's crop coordinates. Inspect actual nontransparent bounds for every frame, preserve the full chest feet and raised lid, and add transparent padding on all sides. Align all twelve frames to a shared floor baseline and pivot without per-frame scaling. Verify every frame individually and in playback; the corrected crop exports have not yet been produced. The preview used simulated glow/sparkles and a placeholder sword, not the final runtime effects or equipment renderer.

Use current-game-chest-reference.png supplied by the user as the authoritative appearance reference. The earlier embedded repository thumbnail was visually inconsistent with the current game and must not guide the chest design. Match warm brown wood, dark iron bands, gold hardware and golden treasure light. Preserve the actual chest's appearance and camera. Correct any geometry drift in the opening lid. Manually crop frames from the source sheets, remove colored edge noise, and place on uniform padded canvases. Keep a fixed chest floor pivot and constant body size; do not resize each pose independently. Normalize VFX pivots by floor anchor. Verify alpha over both light and dark backgrounds; zero corner alpha alone does not establish clean transparency. Preserve originals. Rebuild loops where necessary.

Use existing pixel-art texture filtering and appropriate alpha-capable material. Do not apply the magenta chromakey material blindly to RGBA effects. Keep floor glow behind the dropped equipment icon, sparkles above it, and the chest release burst at the chest mouth. Do not bake item art or rarity into the chest frames.

## Existing behavior to change

DungeonActors.cpp currently offers three mystery chests, rolls a selected chest once into ChestLoot[Choice], and immediately calls AddToInventory. A full inventory retains the rolled reward. Successful collection heals 35 health, plays Chest audio, hides chests, sets bLootClaimed and opens room progression after a cooldown. Preserve the one-choice reward policy, rarity distribution, rolled stats, and no-duplicate guarantees while changing the visual flow.

## Desired state flow

Closed -> Opening -> Ejecting -> AvailableOnFloor -> Collected.

On a valid interaction, lock the selected chest and disable the other two choices. Roll only once and retain the full FDungeonItem instance. Opening the chest must NOT immediately grant inventory loot. Guard repeated input during all animation states.

Suggested starting timing: opening frames at 16 fps (about 0.75 seconds). At frame 8, emit the separate release burst and spawn one floor-loot instance containing the already rolled item. Ensure the release event executes exactly once even when a large frame delta crosses its time. Hold the final open chest pose after the sequence.

Animate the existing item icon from the chest mouth toward a nearby navigable floor point over about 0.45 seconds. Use a fixed logical landing point and interpolate the screen/world-plane path, with a separate vertical visual offset h*sin(pi*t). Keep the shadow/glow anchored on the floor; the icon alone rises. Avoid mixing screen coordinates with world height in this project's projected renderer. Add a small damped landing bounce and play landing VFX once. Resolve destinations away from walls, doors, and other props; never leave the reward unreachable.

Start the appropriate rarity idle glow on landing. Suggested initial loop rate is 6 fps, tuned in playback. Match rarity indices 0..4 and RarityColor: (.65,.69,.72), (.3,.88,.53), (.3,.62,1), (.8,.38,1), (1,.6,.16). Keep the actual item visible, with restrained glow opacity. Label item/rarity on focus so color is not the only cue.

Pickup uses the existing E interaction within range once the item lands. On AddToInventory success only, mark collected, remove the floor item and its glow, emit pickup sparkles once, and run the existing heal/progression behavior exactly once. If inventory is full, keep the same item on the floor with unchanged rolled stats and show the existing bag-full guidance; no pickup sparkle or reward consumption. Do not open room exits while required loot is still unclaimed unless a deliberate leave-loot design is added.

Pause presentation with gameplay pause/dialogue/inventory as appropriate to existing rules. Clear pending events and floor loot safely on restart/room teardown; no delayed spawn into a new room. Keep the current Chest sound timed to opening; do not double-play it at collection. Pickup audio can reuse an appropriate existing cue until original audio is supplied.

## Validation

Check all five rarities and weapon/armor/amulet icons; one choice among three chests; repeated E and low frame rates; inventory full at opening and at pickup; making space then collecting the same rolled item; room transitions/restarts during animation; exactly one spawn, inventory grant, heal, and progression event. Update existing chest smoke checks that currently assume immediate inventory delivery so they advance the sequence and explicitly pick up floor loot. Verify glow readability, lid continuity, frame pivots, and all one-shot/loop seams in actual playback.

