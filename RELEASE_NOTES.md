# v0.1.0 — Windows playtest

Download `BeardAndBlade-Windows-v0.1.0.zip`, extract the entire archive, then open `Windows/BeardAndBlade.exe`. UE5 is not required. Keep every supporting folder together. Windows x64; unsigned early playtest build.

- Four dungeon themes, staggered waves, bosses every fourth room; Finance Guy is the first boss.
- Three choose-one reward chests, equipment inventory and animated room transitions.
- Sword combat, sprinting, dodge roll and a ten-second-cooldown tea splash power move.
- Original synthesized menu, dungeon and boss music; 16 gameplay effect cues.

Controls: WASD movement; Shift sprint; Space dodge; mouse aim; LMB sword; RMB tea; E interact; I inventory; P menu; M music on/off; N effects on/off. Audio preferences are saved locally.

Verification: UE 5.8 Shipping build/cook/stage succeeded. Standalone packaged executable automated smoke test passed, including all 19 audio assets, menu-to-game transition, active music component and tea cooldown. Menu/gameplay screenshots were inspected. Existing automated campaign tests passed. No full human balance/audio-listening playthrough is claimed.

Known limitations: no save system or campaign ending; themes repeat after room 16; effects share sound families; no browser build, installer or automatic updater. This release is for feedback, not a finished commercial release. If prerequisites are missing, use the bundled redistributable under Engine/Extras/Redist/en-us. Only run downloads from trusted sources.
