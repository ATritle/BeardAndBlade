"""Check that both Windows executables embed the project's exact icon data."""
import ctypes
from ctypes import wintypes
from pathlib import Path
import struct
import sys

root = Path(__file__).resolve().parents[1]
ico = (root / 'Build/Windows/Application.ico').read_bytes()
count = struct.unpack_from('<H', ico, 4)[0]
expected = set()
for i in range(count):
    length, offset = struct.unpack_from('<II', ico, 6 + i*16 + 8)
    expected.add(ico[offset:offset+length])
k = ctypes.WinDLL('kernel32', use_last_error=True)
k.LoadLibraryExW.argtypes = [wintypes.LPCWSTR, wintypes.HANDLE, wintypes.DWORD]
k.LoadLibraryExW.restype = wintypes.HMODULE
k.FindResourceW.argtypes = [wintypes.HMODULE, ctypes.c_void_p, ctypes.c_void_p]
k.FindResourceW.restype = ctypes.c_void_p
k.LoadResource.argtypes = [wintypes.HMODULE, ctypes.c_void_p]
k.LoadResource.restype = ctypes.c_void_p
k.LockResource.argtypes = [ctypes.c_void_p]
k.LockResource.restype = ctypes.c_void_p
k.SizeofResource.argtypes = [wintypes.HMODULE, ctypes.c_void_p]
k.SizeofResource.restype = wintypes.DWORD
k.FreeLibrary.argtypes = [wintypes.HMODULE]
callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HMODULE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_ssize_t)
k.EnumResourceNamesW.argtypes = [wintypes.HMODULE, ctypes.c_void_p, callback_type, ctypes.c_ssize_t]
for exe in sys.argv[1:]:
    module = k.LoadLibraryExW(str(Path(exe).resolve()), None, 2)
    if not module:
        raise ctypes.WinError(ctypes.get_last_error())
    actual = set()
    @callback_type
    def collect(handle, kind, name, param):
        resource = k.FindResourceW(handle, name, kind)
        pointer = k.LockResource(k.LoadResource(handle, resource))
        actual.add(ctypes.string_at(pointer, k.SizeofResource(handle, resource)))
        return True
    k.EnumResourceNamesW(module, 3, collect, 0)
    k.FreeLibrary(module)
    assert expected <= actual, f'Custom icon missing from {exe}'
    print(f'PASS: {exe}: all {count} custom icon resolutions embedded')
