# Boss UI source pack v1

Created with built-in image_gen. Exact prompts are in generation-prompts.json.

## Contents and roster

- current-bosses-ui.png: 1536 x 1024 RGBA; top-to-bottom Finance Guy, The Webroot Matriarch, The Rime Empress, The Cinder Warden. These are the four runtime bosses in DungeonRoster.h at repository commit 9760aecc21cd7b92261b2d4d4339890db7b21d19.
- ../v2/big-mack-twister-ui.png: 2172 x 724 RGBA; Big Mack above Twister. These two have source art in the repository but are not yet in the runtime roster.
- Each row has a portrait on the left, name artwork in the middle, and empty health-bar border on the right.

The four existing boss portraits are NEW THEME-BASED DESIGNS. Their existing binary Unreal textures could not be previewed through the repository connector, so compare these portraits against the actual game sprites at home before approving character identity. Big Mack and Twister use their earlier reference images.

## Cleanup and integration prompt

Inspect these source sheets in the UE workstation art editor. Crop the 18 components individually with transparent padding; this is an illustrated layout, not a uniformly sliced sprite atlas. A few ornaments approach adjoining rows, so use manual bounds. Retain originals. Inspect alpha against black, white and checkerboard: PNG corner alpha is zero, but glows/fringes need inspection and cleanup. Ensure each health-bar opening has zero alpha. Do not chromakey these images using the magenta sprite material.

Export clean individual PNGs for each boss: portrait, name, health border. Match portrait display size and preserve aspect ratio. Import with UI texture settings and evaluate nearest-neighbor filtering at intended scale. Use a UI material that reads texture alpha where necessary.

Build a reusable boss HUD with separate background track, runtime clipped health fill, damage-lag fill, and decorative border on top. Use boss-specific interior bounds; generated border openings are NOT identical. Mask fill to the opening so it never spills outside ornaments. Do not scale or stretch the full illustrated border with current health. Clamp health fraction to [0,1], guard zero max health, and hide on encounter completion. Keep the name artwork separate so it can scale independently.

Add the portrait to the speech-bubble speaker slot. Existing dialogue uses a 320 x 124 bubble on a 1280 x 800 virtual canvas. Expand/reflow the text region for the portrait rather than overlapping dialogue or Skip/Fight controls. Keep rounded corners and speaker-tracking tail. Use the active speaker's portrait, and retain a text name in accessibility/tooltip data even though the visible title uses artwork.

Verify all six identities and name spellings, long dialogue, resolution scaling, full/half/empty health, and health loss animation. These are source graphics, not installed Unreal assets; no UE runtime testing has been performed on the work PC.

