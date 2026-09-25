# v0.3.1 — boss entrances and campaign endings

Download `TheBeardAndBlade-Windows-v0.3.1.zip`, extract the entire archive, and run `TheBeardAndBlade/TheBeardAndBlade.exe`. Windows x64; Unreal Editor is not required.

- Illustrated six-second entrance cinematics for all seven bosses, with separate character/title art, themed debris, impact timing and synchronized audio.
- Click, Space or Enter to skip an entrance; P pauses and R toggles reduced intro motion. Boss dialogue follows before combat resumes.
- New illustrated death screen, chamber reached, a closing saying, and New Run/Exit buttons.
- New illustrated victory screen and closing story after Twister in chamber 21. The campaign no longer loops into room 22. Earlier rooms retain their chest rewards.
- Safe ending cleanup, one-second restart input guard, and fresh-run resets.

Controls remain WASD, Shift sprint, Space dodge, LMB attack, RMB tea, MMB FREEDOM, E interact, I inventory, P pause, M music and N effects. Enter restarts from an ending. Editor-only boss shortcuts are disabled in Shipping.

Unsigned Windows build; no saved campaign progress. Flash grenades use bright fading effects and blur. This download is not yet configured for StreamPixel hosting; a separate Pixel Streaming-enabled package is needed.

## Earlier releases

# v0.3.0 — seven-chapter dungeon campaign

Download `TheBeardAndBlade-Windows-v0.3.0.zip`, extract the entire archive, and run `TheBeardAndBlade/TheBeardAndBlade.exe`. Windows x64; UE5 is not required. Keep the included folders alongside the executable. Visual C++ prerequisites are included under `Engine/Extras/Redist/en-us`.

- Twenty-one rooms: bosses every third room in order Finance Guy, Big Mack, Flash Bang Guy, Webroot, Rime, Cinder, Twister.
- New Greaseworks kitchen arena; matching bunker and storm chapters; 18 new animated enemies with melee, charge, rifle, grenade and elemental attacks.
- Sharper 256-pixel enemy frames, nearest-neighbor UI textures without mip streaming, frame preloading, and opaque spawning instead of washed-out sprite fades.
- Flash Bang Guy follows a successful stun from his own grenade with a teleport, one knife strike for exactly 25% of the player's current health at impact, and a teleport back. Looking away avoids both flash and combo. Ordinary flash troops cannot trigger this boss attack.
- Updated boss art, health-bar fit, weapon grip corrections, player idle breathing and Twister projectile/audio improvements from the local playtests.
- Animated chest opening, actual-item ejection, rarity glow, floor pickup and full-inventory protection.
- UE editor test shortcuts use the number row above QWERTY; test-room shortcuts are not enabled in the Shipping release.

Controls: WASD move; Shift sprint; Space dodge; LMB attack; RMB tea; MMB FREEDOM; E interact/pick up; I inventory; P menu; M music; N effects.

Unsigned Windows build. No saved campaign progress or ending; the seven-chapter rotation repeats after room 21. Flash grenades produce a bright fading flash and temporary blur. Balance and animation feel remain subject to player feedback.

## Earlier releases and development history

# v0.2.1 — combat balance and HUD cleanup

Extract the complete `TheBeardAndBlade-Windows-v0.2.1.zip` and launch `Windows/TheBeardAndBlade.exe`. Windows x64; Unreal Editor is not required.

- Removed the leftover upper-left attack/armor icons and room-state portal/chest/enemy icon.
- Ordinary enemy potion drop chance reduced from 35% to 12%. Bosses still guarantee a potion; healing amount is unchanged.
- Normal enemy melee, projectile and charge damage increased by about 36%. Boss damage is unchanged.
- Slowed walking to about 8 animation frames/sec and sprinting to about 10. Sprint movement remains 2× speed.
- Footsteps follow the slower gait cycle; reduced chest pulsing and vertical bob.
- Bleed and poison each have a 10% chance per successful weapon hit. Tooltips show the chance. Their existing durations and damage remain unchanged.
- Armor retains its stats but no longer changes the player's starting outfit colors, including during rolls.

Controls: WASD move; Shift sprint; Space dodge; LMB attack; RMB tea; MMB FREEDOM; E interact; I inventory; P menu; M music; N effects.

Unsigned feedback build. No saved campaign progress or ending; themes repeat after room 16. Windows only, not a browser build. The Visual C++ prerequisite installer is in Engine/Extras/Redist/en-us/vc_redist.x64.exe if needed.
# Local v0.3.0 playtest — September asset integration

- Big Mack in room 8; Twister in room 24; six-boss rotation preserves all previous bosses.
- Eight-direction boss animation, burger impacts/debris/landing dust, timed stun/slow, single-shot and full-auto attacks.
- Illustrated boss portraits/nameplates, dynamic ornate health bars and animated status indicators.
- Animated chest opening, actual-item ejection, rarity glow, floor pickup and full-bag protection.
- Original source sheets retained with reproducible cleaned exports. See SEPTEMBER_INTEGRATION.md for balance and art caveats.
- Local Windows package only; this task does not publish a new GitHub release.
