# v0.2.1 validation — September 23, 2026

- Editor and Shipping builds passed; fresh v0.2.1 cook/stage/archive passed.
- Campaign suite passed with added movement/cadence checks and 400-hit low-proc validation.
- Both startup and loot/inventory smoke tests passed with errors=0 from a fresh extraction of the final ZIP.
- Equipped-armor render gallery confirms all armor retains the starting outfit. Packaged gameplay capture checked for upper-left HUD removal.
- Potion chance is 12% for ordinary enemies; bleed/poison 10% per landed hit; normal enemy damage multiplier increased from 1.32 to 1.8. Boss values and potion healing amount unchanged.
- Animation cadence is retimed using the existing six walk poses, not a new animation atlas. Subjective movement/audio feel and difficulty still need player feedback.

## Previous v0.2.0 validation

- UE 5.8 Development Editor and Windows Shipping targets compiled and linked successfully.
- In-engine campaign suite: DUNGEON_CAMPAIGN_VERIFY_COMPLETE errors=0, exit 0.
- Packaged Shipping loot suite: LOOT_SMOKE errors=0; 49 textures, hover inspection, timed ailment rendering/expiration. Includes bag movement, invalid overlap/out-of-bounds drops, wrong-slot rejection, equipment swap and double-click tests.
- Packaged startup suite: PACKAGED_SMOKE errors=0; 19 base audio assets, menu-to-game transition, tea cooldown and playing music component.
- GPU captures inspected for all 24 equipped weapons facing right and left. Compact weapons are smaller; large swords retain their relative size. Scaling preserves grip pivots and does not change attack range.
- BuildCookRun cook/stage/archive completed successfully. The release was archived into a fresh v0.2.0 directory to exclude an old BeardAndBlade payload found in the former reusable Release folder.
- ZIP integrity was checked with testzip. The ZIP includes the launcher, Shipping executable, cooked content, libraries, x64 redistributable and player instructions. SHA256SUMS.txt accompanies it.
- Source excludes packaged binaries, caches, raw/private image references and local captures. Obsolete preview notes and unused prototype assets were retired into the ignored ArtSource/Retired-v020 directory for recovery.

Automated checks and staged render inspection are not a full human playthrough or cross-machine compatibility test. No subjective audio/balance approval is claimed. The developer campaign run logged sandbox-denied Unreal stored-key writes, but its gameplay checks passed and the process exited successfully.

Known limits: Windows x64 only; unsigned playtest; no saved campaign or ending; themes repeat after room 16; three shared armor appearance families; eagle screech plus graphical FREEDOM callout, not recorded speech.
