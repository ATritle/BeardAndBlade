# Boss intro playtest — local review

All seven bosses now have the same six-second entrance presentation. Big Mack intro-v1 and Twister intro-v5 remain unchanged. Five new character/title pairs were generated with the built-in image-generation tool, using existing boss artwork as identity references. No release or GitHub push was made for this update.

## Review in UE5

Open `TheBeardAndBlade.uproject`, then Play in the selected viewport. Click the viewport to give it keyboard focus.

| Top-row key | Boss | Room |
| --- | --- | --- |
| 4 | Finance Guy | 3 |
| 5 | Big Mack | 6 |
| 6 | Flash Bang Guy | 9 |
| 7 | Webroot Matriarch | 12 |
| 8 | Rime Empress | 15 |
| 9 | Cinder Warden | 18 |
| 0 | Twister | 21 |

- Allow the six-second intro to finish, or click / press Space / Enter to skip it.
- The existing boss dialogue follows; combat remains blocked until the dialogue is dismissed.
- **P** pauses through the existing menu. **R** toggles reduced intro motion (saved between sessions).
- Repeat the boss shortcut to replay its introduction.

Check the entrance and settling motion, title impact/audio timing, themed debris, and transition into dialogue/combat. Please also listen to the synthesized intro cue and judge its level alongside the existing game audio; automated tests run without sound.

## Integration

- Separate character and title images, native transparency, no mip streaming.
- Actual destination dungeon behind the dimmed intro overlay.
- Six-second entrance / settle / title impact / hold / fade sequence.
- Synchronized synthesized cue based on the uploaded browser sketch; respects audio mute settings.
- Pause, skip, reduced-motion mode, restart/death cleanup, and combat gating.
- Each boss retains the existing dialogue after its cinematic; combat balance and room order are unchanged.
- Themed debris: paper/coins, food, bunker dust, forest fragments, ice, embers, and storm rubble.

Editor build and automated intro, campaign, September, flashbang, and progression checks passed. Rendered review captures are `ArtSource/BossReview24.png` through `BossReview30.png` (local-only, ignored by Git; species order is Finance, Webroot, Rime, Cinder, Mack, Twister, Flash).

Runtime assets: `Content/Art/Intros` and `Content/Audio/BossIntroCue.uasset`. The source cue can be regenerated with `Tools/prepare_intro_audio.py`; `Tools/import_boss_intros.py` imports the supplied artwork and cue through UE's Python commandlet.

New source artwork is in `ArtSource/Bosses/{Finance,Flash,Webroot,Rime,Cinder}/intro-v1/`, each containing `character.png` and `title.png`. Generation prompts are saved in `ArtSource/Bosses/INTRO_GENERATION_PROMPTS.md`. These source files are local-only under the existing ArtSource ignore rule; the imported runtime assets are available for the eventual reviewed commit.
