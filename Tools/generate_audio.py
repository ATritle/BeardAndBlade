"""Original deterministic synthesized score/SFX. No samples or third-party recordings."""
from pathlib import Path
import wave,json
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Content/Audio';OUT.mkdir(exist_ok=True)
SR=32000
rng=np.random.default_rng(1947)
report={}
def save(name,a,music=False):
    a=np.nan_to_num(a)
    peak=float(np.max(np.abs(a)))
    if peak: a=a/peak*(.72 if music else .78)
    if a.ndim==1:a=np.column_stack([a,a])
    with wave.open(str(OUT/(name+'.wav')),'wb') as f:
        f.setnchannels(2);f.setsampwidth(2);f.setframerate(SR)
        f.writeframes((a*32767).astype('<i2').tobytes())
    report[name]={'seconds':len(a)/SR,'peak':float(abs(a).max()),'rms':float(np.sqrt(np.mean(a*a))),'loop':music}
def note(midi,dur,kind='bell'):
    t=np.arange(int(SR*dur))/SR;f=440*2**((midi-69)/12)
    if kind=='pad':
        a=(np.sin(2*np.pi*f*t)+.24*np.sin(2*np.pi*f*2.003*t)+.16*np.sin(2*np.pi*f*.998*t))
        env=np.minimum(t/.6,1)*np.minimum((dur-t)/.8,1)*.35
    else:
        a=np.sin(2*np.pi*f*t)+.32*np.sin(2*np.pi*f*2*t)*np.exp(-t*3)+.14*np.sin(2*np.pi*f*3*t)*np.exp(-t*5)
        env=(1-np.exp(-t*150))*np.exp(-t*(2.2 if kind=='pluck' else 1.15))*np.minimum((dur-t)/.04,1)
    return a*env
def score(name,bpm,roots,melody,dark=False):
    beat=60/bpm;n=int(32*beat*SR);a=np.zeros((n,2))
    def add(sound,start,gain,pan=0):
        idx=(np.arange(len(sound))+int(start*SR))%n
        for c,v in enumerate([np.sqrt((1-pan)/2),np.sqrt((1+pan)/2)]):np.add.at(a[:,c],idx,sound*gain*v)
    for bar in range(8):
        root=roots[bar%len(roots)]
        for interval in [0,7,15]:add(note(root+interval,beat*4.8,'pad'),bar*4*beat,.32,interval/30-.25)
        for step in range(8):
            pitch=root+12+[0,7,12,15,12,7,10,7][step]
            add(note(pitch,beat*1.8,'pluck'),(bar*4+step*.5)*beat,.09 if dark else .16,(-1)**step*.4)
        for step in range(2):
            pitch=melody[(bar*2+step)%len(melody)]
            add(note(pitch,beat*3), (bar*4+step*2)*beat,.13 if dark else .22,.12)
        if dark:
            for step in [0,2]:
                t=np.arange(int(.3*SR))/SR
                drum=np.sin(2*np.pi*(54*t+18*(1-np.exp(-t*15))/15))*np.exp(-t*17)
                add(drum, (bar*4+step)*beat,.23)
    # Circular short echoes retain the exact musical loop boundary.
    a+=.18*np.roll(a,int(.375*beat*SR),axis=0)+.09*np.roll(a,int(.75*beat*SR),axis=0)
    save(name,a,True)
score('MusicMenu',76,[38,34,41,36],[62,69,65,64,62,60,57,60],False)
score('MusicDungeon',68,[38,38,34,36],[50,57,53,52,50,48,45,48],True)
score('MusicBoss',104,[38,39,34,36],[62,63,69,65,62,60,58,57],True)
def fx(name,dur,kind):
    t=np.arange(int(dur*SR))/SR;noise=rng.normal(0,1,len(t))
    smooth=np.convolve(noise,np.ones(9)/9,mode='same')
    env=np.minimum(t/.008,1)*np.minimum((dur-t)/.03,1)
    if kind=='swish':a=(noise-smooth)*np.sin(np.pi*t/dur)**2*.45
    elif kind=='roll':a=smooth*np.exp(-t*7)+.18*np.sin(2*np.pi*85*t)*np.exp(-t*12)
    elif kind=='metal':a=sum(np.sin(2*np.pi*f*t)*np.exp(-t*d) for f,d in [(430,12),(1079,18),(1811,22)])*.23+noise*np.exp(-t*40)*.25
    elif kind=='boom':a=smooth*np.exp(-t*4)*1.6+np.sin(2*np.pi*(65*t+4*np.sin(t*6)))*np.exp(-t*7)*.4
    elif kind=='water':a=smooth*(.4+.6*np.sin(t*60)**2)*np.exp(-t*5)+np.sin(2*np.pi*(900*t-350*t*t))*np.exp(-t*18)*.15
    elif kind=='paper':a=(noise-smooth)*(.2+.8*np.sin(t*57)**8)*np.exp(-t*8)
    elif kind=='magic':a=(np.sin(2*np.pi*(420*t+330*t*t))+.4*np.sin(2*np.pi*840*t))*np.exp(-t*5)*.3+smooth*.1
    elif kind=='step':a=smooth*np.exp(-t*45)+np.sin(2*np.pi*95*t)*np.exp(-t*60)*.25
    elif kind=='hurt':a=np.sin(2*np.pi*(160*t-65*t*t))*np.exp(-t*9)*.5+smooth*np.exp(-t*14)
    elif kind=='death':a=np.sin(2*np.pi*(130*t-45*t*t))*np.exp(-t*4)*.4+smooth*np.exp(-t*6)
    elif kind=='chime':a=sum(note(p,.9)[:len(t)] for p in [62,66,69])*.23
    save(name,a*env)
for args in [('Sword',.28,'swish'),('Roll',.42,'roll'),('Hit',.24,'metal'),('Explosion',.85,'boom'),('TeaSplash',.65,'water'),('Throw',.24,'swish'),('Paper',.34,'paper'),('Magic',.5,'magic'),('Step',.13,'step'),('Hurt',.35,'hurt'),('Death',.85,'death'),('Chest',.9,'chime'),('Equip',.2,'metal'),('Portal',.65,'magic'),('UI',.12,'metal'),('Spawn',.45,'magic')]:fx(*args)
(OUT/'audio_manifest.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
