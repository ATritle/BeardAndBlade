# Death and campaign victory screens — local review

## What changed

- Death now fades into an illustrated fallen-adventurer scene with the saying: “The tea went cold. The legend didn't.” It also shows the chamber reached.
- Defeating Twister in chamber 21 now ends the campaign with a victory illustration and closing story. There is no final chest or gateway to chamber 22. Earlier bosses still award their normal chests.
- Both screens have ornate **New Run** and **Exit** buttons. Enter also starts a fresh run after the one-second input guard.
- Combat, spawning, inventory actions, residual projectiles, flash effects, and transitions stop at the ending. Pause cannot resume a finished run. New Run resets health, inventory, abilities, room progression, and ending state.
- The existing menu music is used on both ending screens, respecting mute settings.

## Review

Open `TheBeardAndBlade.uproject` and Play. Let the adventurer die to review the death screen. Use top-row **0** for the Twister playtest, then defeat him to review campaign victory.

For automatic visual captures, run `Tools/capture_review.ps1 -Preview Death` or `-Preview Victory`. These explicit review sessions close automatically after saving their screenshots.

Rendered captures: `ArtSource/DeathEndingReview.png` and `ArtSource/VictoryEndingReview.png`.

## Artwork and verification

Two new scenes were created with the built-in image-generation tool, using the current menu adventurer as the identity/style reference. The titles are part of the artwork; story text remains separately rendered for readability.

Source images: `ArtSource/Endings/Death.png` and `ArtSource/Endings/Victory.png`. Final prompts: `ArtSource/Endings/PROMPTS.md`. Runtime assets: `Content/Art/Endings/EndingDeath.uasset` and `EndingVictory.uasset`. Import script: `Tools/import_endings.py`.

Editor build succeeded. Ending, intro, full campaign, September combat, flashbang, and progression verification suites passed with zero errors. Both screens were rendered in UE and visually inspected. The ending tests cover real final-boss damage, earlier boss rewards, death, input guarding, gameplay blocking, restart cleanup, and preventing room 22.

Nothing has been committed, pushed, packaged, or published for this update.
