# September artwork integration — local v0.3.0 playtest

Based on asset upload d830fc196bf6dd59c2b517f02aa5f7ca6566d61d. Original artwork retained. Existing untracked social-preview files preserved. This update is local; no GitHub release is published by this task.

## Campaign and provisional balance

Boss order approved by the user: Finance Guy (4), Big Mack (8), Webroot Matriarch (12), Rime Empress (16), Cinder Warden (20), Twister (24). The six-boss rotation repeats; existing four-theme cadence remains. There is no new ending/save system.

Big Mack: 900 base HP, 32 projectile damage before armor, burger speed 300, 0.75-second stun and 3-second 40% slow. Reapplications refresh to at least those durations, never add duration. Alternate 0.95-second hops and 0.55-second throws with 0.65-second recovery. Landings are cosmetic dust, not additional damage or warning circles.

Twister: 1600 base HP, 24 bullet damage before armor, speed 500, lifetime 2.5 seconds. Two normal shots alternate with a finite ten-round burst, 0.12 seconds between rounds after a 0.55-second windup. Burst recovery 1.6 seconds; normal recovery 0.75 seconds. Existing room HP scaling remains. Rifle audio temporarily reuses the pitched Hit cue; no recorded rifle/tornado voice supplied.

## Artwork

`Tools/prepare_september_art.py` preserves inputs and exports cleaned RGBA images to `Content/Art/September`, with `frames.json` crop/pivot metadata and `health-openings.json`. Alpha cleanup removes low-opacity source haze, not magenta. Source dimensions and transparent gutters are measured; chest rows use separately inspected boundaries. All chest poses share a padded 352-square canvas and floor baseline with no per-frame resizing. Twister uses a padded 384-square canvas and a funnel-tip anchor, excluding low muzzle flashes from pivot calculation.

Southwest Big Mack throw poses 12–15 had right-facing drift: corrected using mirrored matching southeast throws. The remaining southwest poses remain from that direction's source. Stun's orbit is procedural rather than trusting the uneven generated star arrangements. Existing sprite frames still contain drawn silhouette/weapon differences; these are generated pose animations, not rigged motion capture.

Boss UI components are separately cropped. Fill rectangles use measured, cleared openings and damage-lag fill. Finance Guy's source portrait materially changed his identity (glasses/slick hair/villain face); the runtime portrait retains his original sprite face/body within the supplied frame. The other original-boss portraits remain theme interpretations: Webroot's UI is more violet than the green creature, and Cinder's portrait is a closed helmet versus the existing horned fiery body. Their gameplay sprites were not redesigned. Further portrait painting may be desirable.

## Reward invariants

Closed → Opening → Ejecting → Available → Collected. Choice locks and rolls once, release starts at 0.5 seconds, landing at 0.95 seconds. E picks up within 85 floor units. Inventory insertion is the gate for healing, progression and pickup sparkles. Full bags leave the exact rolled item on the floor. Animations pause with inventory/menu/dialogue. Room/restart resets pending state. Closed chests stay gold/brown; rarity affects only the dropped-item glow.

## Validation commands

Editor `-game -SeptemberVerify -nullrhi`: directional texture coverage, full-bag/repeated-input/low-frame-rate loot tests across all rarities and equipment slots, status duration/movement/immunity, projectile origins, burst counts at 0.016/0.2/2-second deltas, and boss order.

Editor `-game -DungeonVerify -nullrhi`: expanded existing 24-room campaign regression plus combat, stamina, inventory, effects, tea and Freedom.

Editor or packaged game `-DungeonSeptemberSmoke`: rendered six-boss introductions/combat, chest choice/open/eject/land/full bag/collection; screenshots and `SeptemberSmokeTest.txt` under Saved. Existing `-DungeonSmokeTest` and `-DungeonLootSmoke` remain available.

## Distribution

Editor and Shipping builds completed. Focused September verification and the 24-room campaign regression both reported zero errors. The staged Shipping EXE passed September, loot, and general/audio-component smoke tests with zero errors; its executable SHA-256 matches the compiled Shipping binary. These automated checks do not replace a human balance or audio-listening pass.

Fresh local staging: `Builds/v0.3.0-playtest/Windows`. ZIP entry point: `TheBeardAndBlade/TheBeardAndBlade.exe`. Keep every supporting folder; Windows x64 only, unsigned playtest, Unreal Editor not required. Do not distribute only the inner shipping executable.
