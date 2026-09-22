"""Archive a complete staged Windows build; omit debug symbols and user data."""
from pathlib import Path
import zipfile,hashlib
root=Path(__file__).resolve().parents[1]
build=root/'Builds/Release/Windows'
assert (build/'BeardAndBlade.exe').is_file()
assert list(build.rglob('*.ucas'))
out=root/'Builds/BeardAndBlade-Windows-v0.1.0.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for file in sorted(build.rglob('*')):
        if not file.is_file() or file.suffix.lower() in ('.pdb','.log') or 'Saved' in file.relative_to(build).parts:continue
        z.write(file,Path('Windows')/file.relative_to(build))
with zipfile.ZipFile(out) as z:
    assert z.testzip() is None
checksum=hashlib.file_digest(out.open('rb'),'sha256').hexdigest()
(out.parent/'SHA256SUMS.txt').write_text(f'{checksum}  {out.name}\n')
print(out, out.stat().st_size, 'bytes; SHA256',checksum)
