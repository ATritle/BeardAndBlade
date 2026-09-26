"""Archive a complete staged Windows build; omit debug symbols and user data."""
from pathlib import Path
import zipfile,hashlib,argparse
parser=argparse.ArgumentParser()
parser.add_argument('--version',default='v0.3.2')
parser.add_argument('--streaming',action='store_true',help='Use StreamPixel Windows layout and hosting checklist')
args=parser.parse_args()
assert all(c.isalnum() or c in '.-_' for c in args.version)
root=Path(__file__).resolve().parents[1]
build=root/'Builds'/args.version/'Windows'
assert (build/'TheBeardAndBlade.exe').is_file()
assert list(build.rglob('*.ucas'))
assert not (build/'BeardAndBlade').exists(), 'Old project payload must not ship'
assert not (build/'BeardAndBlade.exe').exists(), 'Old launcher must not ship'
assert (build/'Engine/Extras/Redist/en-us/vc_redist.x64.exe').is_file()
prefix='TheBeardAndBlade-StreamPixel' if args.streaming else 'TheBeardAndBlade-Windows'
archive_root=Path('Windows' if args.streaming else 'TheBeardAndBlade')
out=root/'Builds'/f'{prefix}-{args.version}.zip'
assert not out.exists(), 'Choose a new version; preserve existing player archives'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for file in sorted(build.rglob('*')):
        if not file.is_file() or file.suffix.lower() in ('.pdb','.log') or 'Saved' in file.relative_to(build).parts:continue
        if file.name.startswith('Manifest_'):continue
        z.write(file,archive_root/file.relative_to(build))
    docs=('STREAMPixel_SETUP.md',) if args.streaming else ('README.md','RELEASE_NOTES.md','PROGRESSION_PLAYTEST.md','FLASH_BANG_PLAYTEST.md')
    for name in docs:
        z.write(root/name,archive_root/name)
with zipfile.ZipFile(out) as z:
    assert z.testzip() is None
checksum=hashlib.file_digest(out.open('rb'),'sha256').hexdigest()
out.with_suffix('.sha256').write_text(f'{checksum}  {out.name}\n')
print(out, out.stat().st_size, 'bytes; SHA256',checksum)
