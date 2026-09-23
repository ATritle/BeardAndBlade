# Big Mack combat effects — source pack v1

Created with built-in image_gen. Includes four source sheets matching Big Mack's pixel-art food palette. No ground-warning circles or targeting markers requested or included. The landing shockwave is a post-impact expanding dust burst.

## Sheets and frame intent

All sheets are 1536x1024 PNGs. Read left-to-right, top-to-bottom.

| File | Layout | Content |
|---|---|---|
| burger-splat.png | 4 columns x 3 rows | 12 poses: compressed burger, initial burst, squash, maximum splat, flying food, falling debris, settling, residue, dissipation |
| lettuce-cheese-debris.png | 6 columns x 4 rows | 24 particle poses: row1 whole lettuce leaf, row2 torn lettuce, row3 floppy cheese slice, row4 cheese strand; six tumble/flex poses each |
| landing-shockwave.png | 4 columns x 3 rows | 12 poses: contact puff, outward jets, widening irregular dust front, dust and stone fragments fading |
| stun-stars.png | 4 columns x 3 rows | 12 source poses for three gold stars around an empty head position |

## QA and cleanup status

Source art only, not engine-ready flipbooks. File inspection confirms RGBA and transparent background samples; nonzero alpha samples were partially transparent. Rendered previews show broad colored haze, so inspect composites on light and dark backgrounds and clean unwanted haze/fringes without losing small debris. Do not assume clean opaque sprite interiors or production-quality alpha. Keep originals intact.

Layouts need inspected extraction and consistent pivots. 1024 is not divisible by three: for 4x3 sheets do not assume square or equal integer cells. Check motion continuity; generated poses do not prove a smooth loop. In the stun sheet some successive arrangements change too little to form a uniform orbit. Prefer extracting individual stars and animating their positions procedurally, or redraw in-betweens before using the atlas directly. The debris sheet can be used as independent particles instead of playing the full atlas as one effect. Playback and in-game readability have not been tested.

No gameplay changes, UE asset imports or sound generation performed. Repository location: ArtSource/Bosses/BigMack/combat-effects-v1. Pull main on the home PC and follow UE5_HANDOFF.md for cleanup and integration. generation-prompts.json preserves all generation prompts, including the first discarded splat attempt.
