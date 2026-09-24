# The Beard and Blade — playtest

## Launch

Extract the complete Windows release ZIP and run Windows/TheBeardAndBlade.exe. Developers can open TheBeardAndBlade.uproject in UE 5.8 and use Play > Selected Viewport.

## Controls and checks

- WASD moves in screen directions; diagonals are normalized. Shift sprints, Space rolls. Both consume stamina.
- Mouse aims; LMB attacks; RMB throws tea with a ten-second cooldown.
- MMB activates FREEDOM after 15 ordinary enemy kills. Ability kills do not recharge it.
- E collects one of three chest rewards or enters an unlocked arch. A full bag retains the rolled chest item.
- I toggles the inventory and pauses combat. Hover to inspect; drag to move items or equip in a matching slot; double-click to equip.
- Weapons occupy 1×2 cells, armor 2×2, amulets 1×1. Invalid/overlapping drops leave items unchanged.
- Unequip returns equipment to a free bag area; discard permanently removes an item for this run.
- P opens the menu; M toggles music; N toggles effects. E after death restarts.

Check each weapon facing both left and right, including attack poses. The equipped size is independent of its inventory footprint and combat reach.

## Campaign

Two staggered waves in ordinary rooms, bosses every third room. The current seven-theme rotation repeats after room 21; see [progression playtest](PROGRESSION_PLAYTEST.md) for the boss order and new enemy shortcuts. Choose one of three random rewards after clearing a room; the other chests disappear. All three gates lead to the next room.

The catalog has 24 weapons, 12 armor pieces and 12 amulets. Rolled stats apply only while equipped. Bleed/poison/chill and other combat bonuses expire or trigger according to their tooltip. Armor changes stats but keeps the starting outfit colors. No save system or campaign ending is implemented.

## Automated checks

Development flag `-DungeonVerify` runs the in-engine campaign suite. Shipping flags `-DungeonSmokeTest` and `-DungeonLootSmoke` test packaged startup/audio/tea and loot/effects/inventory gestures respectively, writing result files in Saved.

`Tools/capture_review.ps1 -Preview Weapons` captures all 24 equipped weapons. Add `-Left` to inspect the opposite direction. Other preview modes include Menu, Inventory, Roster, Chest and Boss. These are staged engine captures, not manual playthroughs.
