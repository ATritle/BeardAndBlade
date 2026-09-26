"""Recover original boss-sheet detail without changing sprite geometry or timing."""
from prepare_campaign_art import *
split('BossesReflow.png','Creature',24,size=384,rows=(1,2,3))
assert len(manifest)==24
(src/'boss_clarity_manifest.json').write_text(json.dumps(manifest,indent=2))
print('Prepared 24 Webroot/Rime/Cinder frames at 384x384')
