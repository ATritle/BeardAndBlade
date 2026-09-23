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
