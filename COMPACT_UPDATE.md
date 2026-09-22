# Compact HUD and mystery chests

- The lower HUD, including both detailed pixel-art orbs, spans 416 of the 1280 virtual pixels: centered within the middle third. It retains health, stamina, tea cooldown and dodge status. Room objectives are now at the upper left.
- Boss introductions use a 320x124 speech bubble beside the current speaker, with word-wrapped text. Click in the game to advance, or use the small Skip / Fight button. Combat remains paused until the conversation ends and the grace period expires.
- No text labels are drawn beneath doors or chests. Use E near a chest or an unlocked glowing doorway, as before.
- All three mystery chests independently use the same nine-item pool: three weapons, three armor pieces and three amulets. Rewards can duplicate across chests. A failed pickup due to a full bag preserves the rolled reward; opening one successfully removes the others. Boss rewards remain legendary.
- Enemy base damage is now 1.32x the original values, up from 1.10x in the stamina update (20% more than that build). This covers melee, charges, projectiles and splash damage. Armor reduction is unchanged. The previous 15% health increase remains.
- Stamina, potion pickups, inventory and custom Windows branding are retained. Only the censored attack quip remains.

## Local test package

`Builds/CompactHUD/Windows/BeardAndBlade.exe`. Keep the entire Windows folder together. Builds and engine caches are intentionally excluded from Git; open the source in UE5.8 or run Tools/package_windows.ps1 to build elsewhere.

## Validation

`-DungeonVerify` checks the 16-room campaign, stamina, healing, dialogue, inventory, mystery-loot pool coverage and one-chest-only behavior. `-DungeonDialogueSmokeTest` captures the compact hero/boss bubbles and checks resumed combat. `-DungeonVitalsPreview` captures partial orb fills. `-DungeonSmokeTest` checks packaged menu/game/audio flow.

Verified on September 22, 2026: Editor and Shipping builds succeeded. Campaign verification, packaged dialogue smoke test and packaged menu/audio smoke test all returned zero errors. Chest/HUD and boss dialogue screenshots were visually inspected.
