# Beard & Blade — four-biome campaign update

Open `BeardAndBlade.uproject` in Unreal Engine 5.8 and choose Play > Selected Viewport. The default play mode is now set to that option. This is an Unreal C++ game using a single Canvas sprite renderer; equipment is layered 2D art, not a skeletal 3D model.

## Controls

- WASD: move in screen directions. Diagonal movement is normalized.
- Hold Shift: sprint at twice walking speed.
- Space: directional dodge roll, with brief invulnerability and a 1.15-second cooldown.
- Enter: begin/resume from the title screen. P: pause/resume the menu. Menu buttons also accept left click.
- Mouse: aim from the adventurer's actual position.
- Left click: a six-pose attack; damage happens on the strike frame.
- Right click: throw a teacup at the cursor, with a 10-second active-gameplay cooldown. The cup splashes once, damaging nearby enemies in a broad area. The cooldown pauses in menus/inventory. Casting is unavailable during sword attacks or rolls.
- E near the chest: collect one random item into the bag (does not auto-equip).
- I: open/close the inventory. Combat, movement and spawning pause while it is open.
- Click a bag item, then EQUIP / SWAP to wear it. Previous gear returns to the same bag space.
- UNEQUIP returns equipped gear to the bag if there is space. DISCARD permanently removes a selected bag item for this run.
- E near any glowing emerald arch: enter the next room.
- E after death: restart the run, resetting equipment and room progress.

## Playable content

Normal rooms have two staggered enemy waves. Three chests become available after every enemy has been defeated: weapon, armor and charm. Choose one with E; successful collection puts the item into the bag, removes the other two chests and unlocks all three gates. A full bag leaves the rolled reward in its chest without rerolling. All gates lead to the next room; branching routes are not implemented. Entering a gate locks combat while the hero walks through it and the scene fades into the next room.

The bag is 6 columns by 6 rows. Weapons occupy 1 column by 2 rows, armor 2 by 2, and charms 1 by 1. Items are automatically placed in the first fitting space; manual dragging, rotation and rearrangement are not implemented. Equipping swaps within a slot, so a full bag can still swap a weapon or armor. Gear and bag contents persist between rooms but not after death or closing the game.

Bosses appear every fourth room. Rooms 1–4 are the Forgotten Keep (Finance Guy); 5–8 Webroot Hollows (Webroot Matriarch); 9–12 Glacial Reliquary (Rime Empress); 13–16 Cinder Foundry (Cinder Warden). The four-theme cycle repeats with increasing enemy health. Boss chest choices guarantee legendary rarity. Finance Guy alternates five fast stock certificates with three slower, larger-splash bonds, and browses a laptop between attacks. Other bosses use venom fans, radial projectiles and slam/bursts; bosses become faster below half health. There is no campaign ending or save system in this build.

Tea deals `55 + 0.8 × Attack` damage once to each eligible enemy near the impact; maximum throw range is 450 virtual pixels, splash radius is 135 with the arena's vertical projection compression. Friendly tea does not hurt the hero. Arrows, fire, ice, venom, spores, spectral bolts, stocks and bonds now have sprite projectiles and impact artwork. These are animated using motion, rotation, expansion and fading rather than separate multi-frame explosion atlases.

Hero menu art and all four outfit color atlases now show the bald head, natural reddish beard and round glasses, without a hat or headphones. Roll art has also been updated. Source sheets and prompts: `ArtSource/TEA_PROMPTS.md`. Runtime art: `Content/Art/V2/Tea_*`, `TeaFX_*`, `Finance_*`, and `TeaTitle`.

There are 24 regular species, six per theme, including bats, wisps, harpies, drakes, spiders, scorpions and ground fighters. Each has its own generated sprite sequence and tuned stats; attack logic shares seven archetypes (melee, bolt, charge, fan, slam, venom projectiles, radial burst). Flying enemies have hovering animation and height offsets; they use the same screen-space arena and damage rules as ground enemies.

Nine original item designs support five rarity tiers: common, uncommon, rare, epic, legendary. Stats change only when equipped, replacing the previous slot's bonuses. Weapons use frame-specific hand anchors and grip pivots. Equipping armor selects a complete worn outfit across all 96 animation poses: silver/blue Sentinel, green/teal Verdant, or charcoal/burgundy Warden. Armor icons appear only in the inventory/HUD, never pasted over the body. Charms retain a small pendant overlay. Health upgrades increase maximum health without healing on repeated swaps; successful chest collection restores some health.

## Artwork and animation

- 48 walk poses and 48 attack poses: eight directions, six frames each.
- 288 generated outfit color frames (three armor variants of all 96 poses), using the existing pose alpha masks through M_WornArmor. The silhouettes are shared; this is 2D costume replacement, not a skeletal armor simulation.
- 28 creature sequences with eight timing slots each: walk/fly, wind-up, strike/impact hold and recovery. The impact hold repeats the prior body frame while projectiles and hit effects render separately. Left-facing enemies mirror their right-facing art.
- 32 dodge frames: eight poses in four directions, plus three matching armor-outfit variants.
- Four dungeon backgrounds, original equipment icons, portal and chest art.
- Illustrated interactive title screen with a younger, slender bald adventurer and natural unbraided red beard.
- Hit flash, sparks, knockback, floating damage, and sword-swing trails.

Frames are isolated PNGs with padding and a shared foot anchor; the renderer never samples a neighboring sheet cell. Existing hero poses use alpha; new creatures and rolls use a magenta-key material. Nearest filtering and no mipmaps preserve pixel edges. Source sheets and generation prompts are in `ArtSource`. Runtime textures are in `Content/Art/V2`.

The visual reference is [Emberville's official presentation](https://store.steampowered.com/app/2295170/Emberville/). All newly generated artwork is original to this project, using the built-in image-generation tool. This build uses authored sprite poses and layered equipment rather than Emberville's assets or engine implementation.

## Validation

The `-DungeonVerify` development launch flag runs campaign checks inside Unreal: menu start/pause, W movement, dodge invulnerability/cooldown, attack dispatch for all 28 species, 16-room progression, fourth-room bosses, theme changes, full-bag chest retention, exclusive chest selection, gate transitions, pickup without auto-equip, inventory capacity/swaps and creature/roll texture availability. It exits with a nonzero code on failure. These checks are not a substitute for a human combat/balance playthrough.

The `-DungeonCapture` development flag saves a real engine screenshot and exits. Combine with `-DungeonMenuPreview`, `-DungeonChestPreview`, `-DungeonRosterPreview`, `-DungeonRollPreview`, `-DungeonInventoryPreview`, `-DungeonEquipmentReview` (optionally `-ReviewLeft`), `-DungeonBossPreview`, or `-DungeonRewardPreview`. Add `-DungeonBiome=0` through `3` to preview a theme. These flags do not execute during normal editor play. Rendered review images are stored in `ArtSource`. Pose galleries and chest previews are staged, not manual gameplay recordings.
