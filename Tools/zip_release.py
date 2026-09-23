"""Archive a complete staged Windows build; omit debug symbols and user data."""
from pathlib import Path
import zipfile,hashlib
root=Path(__file__).resolve().parents[1]
build=root/'Builds/v0.2.1/Windows'
assert (build/'TheBeardAndBlade.exe').is_file()
assert list(build.rglob('*.ucas'))
assert not (build/'BeardAndBlade').exists(), 'Old project payload must not ship'
assert not (build/'BeardAndBlade.exe').exists(), 'Old launcher must not ship'
assert (build/'Engine/Extras/Redist/en-us/vc_redist.x64.exe').is_file()
out=root/'Builds/TheBeardAndBlade-Windows-v0.2.1.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for file in sorted(build.rglob('*')):
        if not file.is_file() or file.suffix.lower() in ('.pdb','.log') or 'Saved' in file.relative_to(build).parts:continue
        if file.name.startswith('Manifest_'):continue
        z.write(file,Path('Windows')/file.relative_to(build))
    for name in ('README.md','RELEASE_NOTES.md'):
        z.write(root/name,Path('Windows')/name)
with zipfile.ZipFile(out) as z:
    assert z.testzip() is None
checksum=hashlib.file_digest(out.open('rb'),'sha256').hexdigest()
(out.parent/'SHA256SUMS.txt').write_text(f'{checksum}  {out.name}\n')
print(out, out.stat().st_size, 'bytes; SHA256',checksum)
