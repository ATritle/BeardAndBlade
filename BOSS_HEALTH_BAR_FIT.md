# Boss health fill alignment

Finance Guy, Big Mack, Webroot, Rime, Cinder and Twister now use fill geometry measured from each existing frame's actual transparent opening. The previous rectangles left visible gaps, especially above/below the fill and at its ends.

The red health, amber damage trail and dark depleted background all share the same measured shape. Curved ends and intruding ornament remain intact. No health values, fight mechanics, frame textures or Flash Bang Guy rendering were changed.

Reproduce the measurements and boundary checks with `Tools/measure_boss_health_windows.py`. This emits `Source/TheBeardAndBlade/BossHealthWindows.h` and the QA measurement manifest. Regenerate after replacing any of these six frame textures.

In-engine comparison: `Tools/capture_review.ps1 -Preview HealthBars`. It displays each boss at full health and at 35% health with a 55% damage trail, using the actual HUD renderer. Normal gameplay never runs this review mode.
