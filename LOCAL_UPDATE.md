# Local dialogue and branding update

Initial dialogue and branding update notes. The later compact HUD update supersedes the UI sizes and attack-quips listed below; see COMPACT_UPDATE.md.

## Play

Open `BeardAndBlade.uproject` in UE and use Play, or run
`Builds/LocalDialogue/Windows/BeardAndBlade.exe`.
Keep the complete Windows folder together; the EXE alone is not the game.

- Accepted sword attacks have a 35% chance of a comic quip, with a four-second cooldown. The three lines are “Oi!”, “Take this you C*NT!”, and “Blimey!”.
- Bosses in rooms 4, 8, 12 and 16 each have a six-line introduction. Movement, damage, projectiles, dodge and attacks pause during it.
- After the initial 0.8-second settling period, left-click in the game to advance. The final click closes the dialogue, or click SKIP / FIGHT. Combat begins after a 0.75-second grace period; the dismissing click never attacks.
- P still opens the pause menu during conversations; resuming preserves the current line.
- The Windows launcher and inner executable use a custom seven-resolution icon. The new logo is the standalone launch splash and is included in the package's Branding folder. The existing main menu artwork is preserved.

## Art

Built-in image generation produced `Branding/Icon.png` and `Branding/Logo.png`.
Full prompts are recorded in `Branding/PROMPTS.md`.
`Tools/prepare_branding.py` converts these to the ICO and BMP formats used by Unreal.

## Verification commands

Validated locally: Editor and Windows Shipping builds succeeded; the 16-room campaign suite returned zero errors; packaged dialogue smoke test returned zero errors and its boss/reply/quip screenshots were visually inspected. Both executable icon resources match all seven source ICO resolutions. Launch splash is present in the packaged Content/Splash folder.

- Editor launch with `-game -DungeonVerify -nullrhi -unattended`: 16-room campaign and dialogue pause/advance/skip tests.
- Packaged launch with `-DungeonSmokeTest -unattended`: audio, menu, gameplay and tea cooldown checks.
- Packaged launch with `-DungeonDialogueSmokeTest -unattended`: dialogue, reply, skip, resumed attack and quip screenshots.
- `Tools/verify_windows_icon.py`: compares embedded executable icon resources with the source ICO.
