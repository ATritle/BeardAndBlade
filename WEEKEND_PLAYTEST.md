# Weekend combat update — local review

Based on GitHub main c2a40b4. No GitHub release or StreamPixel deployment is changed by these local edits.

## Balance pass

- Melee uses a 100px weapon reach plus a separate 32–52px standard-enemy hurt radius (up to 80px for bosses), with a forward swing wedge. Enemy attacks/contact ranges are unchanged.
- Standard enemy health is 25% higher than before. Existing 3.5% per-room scaling and boss health are preserved. A Crypt Guard starts at 63.25 HP versus 50.6 previously; a Tomb Rat starts at 34.5 versus 27.6. This is a first-pass tuning value, not a claim that subjective balance testing is complete.
- Moving enemy bolts travel at 80% of their previous speed, capped at 650px/s. Aimed bolts steer for at most 0.55 seconds, at 0.8 radians/s (~46 degrees/s). Rolling, passing the target, or timing out permanently stops steering. Radial barrages/fire cones retain their spread; lobbed grenades retain telegraphed landing points.
- FREEDOM removes 75% of current health, except targets already strictly below 25% maximum health are executed. Exactly 25% survives at 6.25%. Bosses take no damage and show the existing animated immunity/shield status for 2.5 seconds. Pending enemies and waves are no longer skipped.
- Dodge costs 22 stamina (was 30); sprint costs 18/s (was 25/s). Recharge and exhaustion rules are unchanged.

## Art and music

Finance Guy's original sheet is re-extracted into 384px frames rather than throwing away detail into 128px frames; the other 24 original regular-enemy types use 256px frames. Screen size and pivots are preserved. Point filtering, UI texture group, no mips, and no streaming avoid soft or delayed textures. These are recovered source details, not newly painted animations. The 18 newer theme enemies already use higher-resolution frames.

Selected Forgotten Keep MP3 converted to 48kHz stereo PCM, full 132.54-second duration retained, with 1.5-second entry / 3-second exit fades. This is a soft loop boundary, not a beat-matched seamless loop. Existing runtime music crossfades remain. The old synthesized menu music is preserved as MusicEnding; dungeon/boss music is unchanged. Source license documentation still needs to be retained by the owner before redistribution.

## Review in UE5

Open TheBeardAndBlade.uproject and Play in the selected viewport. Start a fresh run. Test attacks without walking into the enemy, including diagonal and upward swings. Compare one-hit/light enemies and armored enemies across early and late rooms. Try dodging across an incoming bolt shortly before impact; it must not turn back afterward. Review stamina recovery between encounters.

Editor number-row shortcuts remain available for chapter/boss testing. Review Finance Guy at ordinary viewport scale, not enlarged screenshots. Listen to a full menu track wrap, and test menu/game transitions and mute/unmute. Final subjective combat feel and hosted audio remain user-review items.

Automated entry point: UnrealEditor.exe TheBeardAndBlade.uproject -game -WeekendVerify -nullrhi -nosound -unattended. Check WEEKEND_VERIFY_COMPLETE errors=0 in the log, not merely process exit status. Existing campaign, progression, flash, September, and ending tests are also retained.
