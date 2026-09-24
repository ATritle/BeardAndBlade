"""Original layered rifle sound design, not a real firearm recording. No third-party samples."""
from pathlib import Path
import wave,json
import numpy as np
root=Path(__file__).resolve().parents[1]; out=root/'Content/Audio'
sr=48000; report={}
for variant in range(3):
    rng=np.random.default_rng(730+variant); t=np.arange(int(sr*.48))/sr
    n=rng.normal(0,1,len(t)); high=n-np.convolve(n,np.ones(13)/13,'same')
    # Sharp muzzle crack, short low-frequency pressure thump, cycling bolt and stone-room reflections.
    dry=high*np.exp(-t/0.009)*.75
    dry+=np.sin(2*np.pi*(145*t+9*(1-np.exp(-t*65))))*np.exp(-t/.033)*.5
    dry+=n*np.exp(-t/.045)*.18
    bolt=np.maximum(t-.047,0); dry+=np.where(t>=.047,high*np.exp(-bolt/.006)*.18,0)
    stereo=np.column_stack([dry,dry])
    for delay,gain in [(.026,.13),(.051,.1),(.089,.055),(.137,.035)]:
        for c in range(2):
            k=int(sr*(delay+c*.002)); stereo[k:,c]+=dry[:-k]*gain
    stereo*=np.minimum(t/.00015,1)[:,None]*np.minimum((.48-t)/.02,1)[:,None]
    stereo=stereo/max(abs(stereo).max(),1e-6)*.78
    name=f'Rifle{variant}'
    with wave.open(str(out/(name+'.wav')),'wb') as f:
        f.setnchannels(2);f.setsampwidth(2);f.setframerate(sr);f.writeframes((stereo*32767).astype('<i2').tobytes())
    report[name]={'seconds':.48,'peak':float(abs(stereo).max()),'rms':float(np.sqrt(np.mean(stereo**2))),'loop':False,'source':'Original procedural rifle sound design'}
(out/'rifle_manifest.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report))
