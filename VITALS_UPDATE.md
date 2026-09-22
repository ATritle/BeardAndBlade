# Local health and stamina update

Initial stamina update notes. The later compact HUD update supersedes the HUD width and enemy damage values below; see COMPACT_UPDATE.md. Use Play in Unreal or package the current source.

- Bottom HUD: red health orb and blue stamina orb with liquid fill and numeric values, ability readiness, room objective and controls.
- 100 stamina. Sprint consumes 25/second while moving; dodge costs 30. Walking and sword attacks remain available without stamina.
- Regeneration starts 0.8 seconds after the last expenditure, restoring 25/second. Reaching zero locks sprint and dodge until the orb fully refills. Otherwise dodge requires at least 30 stamina and its existing cooldown.
- Menu, inventory and boss conversations pause regeneration and pickups.
- Regular enemies have a 35% potion-drop chance; bosses always drop one. Walk over a bottle to restore 35% maximum health, capped at full health. Full-health bottles remain available until leaving the room. Fresh drops have a 0.4-second pickup delay.
- All spawned enemies, including bosses, have 15% more health. Melee, charge, projectile and area attacks do 10% more base damage. Existing armor still reduces damage.
- Removed Oi and Blimey from attack quips and boss dialogue. The remaining attack quip is unchanged.

HUD and potion artwork is custom pixel art generated using the built-in image-generation tool. The dark slate panel is nine-sliced to preserve its carved brass corners. Detailed red/blue liquid textures are clipped to live values within the ornate glass orb frames. Potions have faceted glass, corks, gold fittings, a heart seal, a gentle bob and a floor glow. All five textures use nearest-neighbor filtering without mipmaps.

Source atlas, panel and full generation prompts: `ArtSource/Vitals/`. Imported runtime assets: `Content/Art/V2/HUD_*.uasset`. Preparation and import scripts: `Tools/prepare_vitals_art.py` and `Tools/import_vitals_art.py`.

Automated checks: launch the editor game with `-DungeonVerify` for the campaign and stamina/potion assertions. Packaged `-DungeonVitalsPreview` captures `Saved/Screenshots/VitalsHUD.png` then exits. `-DungeonSmokeTest` checks the normal menu/game/audio flow.

Validation: 16-room campaign, stamina expenditure/exhaustion/refill, pause behavior, potion pickup/overheal prevention and boss drops passed with zero errors. Windows packaged smoke tests passed. The detailed HUD and potion were inspected from a packaged-game screenshot, including partially filled health and stamina orbs.
