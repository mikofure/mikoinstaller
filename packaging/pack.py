#!/usr/bin/env python3
# Minimal packer: bundles a directory tree and metadata into a single self-extracting EXE
# by appending an LZMA-compressed TAR to a bootstrap executable and writing a small trailer.

import argparse
import io
import os
import sys
import tarfile
import lzma
import struct
import time

try:
    import tomllib  # Python 3.11+
except Exception:
    tomllib = None

MAGIC = b"MIKOSETUP\0"
ALGO = b"LZMA"  # 4 bytes
TRAILER_STRUCT = struct.Struct("<Q Q Q")  # (blob_size, meta_size, magic_offset)


def build_metadata(app_name: str, app_version: str, install_dir: str):
    meta = {
        "name": app_name,
        "version": app_version,
        "install_dir": install_dir,
        "created": int(time.time()),
    }
    # Serialize to TOML-like bytes (without external deps); if tomllib unavailable for dump, write simple lines
    lines = []
    for k, v in meta.items():
        if isinstance(v, str):
            lines.append(f"{k} = \"{v}\"\n")
        else:
            lines.append(f"{k} = {v}\n")
    return ("".join(lines)).encode("utf-8")


def add_dir_to_tar(tar: tarfile.TarFile, root: str, base: str = ""):
    base = (os.path.normpath(base) if base else "")
    for dirpath, dirnames, filenames in os.walk(root):
        rel = os.path.relpath(dirpath, root)
        arcdir = (os.path.join(base, rel) if rel != "." else base)
        # Ensure directory entry (skip root when arcdir is empty)
        if arcdir:
            ti = tarfile.TarInfo(arcdir.replace("\\", "/") + "/")
            ti.type = tarfile.DIRTYPE
            ti.mtime = int(time.time())
            ti.mode = 0o755
            tar.addfile(ti)
        for fn in filenames:
            full = os.path.join(dirpath, fn)
            arc = os.path.join(arcdir, fn).replace("\\", "/") if arcdir else fn
            st = os.stat(full)
            ti = tarfile.TarInfo(arc)
            ti.size = st.st_size
            ti.mtime = int(st.st_mtime)
            ti.mode = 0o644
            with open(full, "rb") as f:
                tar.addfile(ti, f)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--project-root", required=True)
    p.add_argument("--build-dir", required=True)
    p.add_argument("--sources-dir", required=True)
    p.add_argument("--bootstrap-exe", required=True)
    p.add_argument("--app-name", required=True)
    p.add_argument("--app-version", required=True)
    p.add_argument("--output", required=True)
    p.add_argument("--install-dir", default=r"%LOCALAPPDATA%\\MikoIDE")
    args = p.parse_args()

    # Prepare tar buffer
    tar_bytes = io.BytesIO()
    with tarfile.open(fileobj=tar_bytes, mode="w") as tar:
        add_dir_to_tar(tar, args.sources_dir, base="")
        # Metadata TOML
        meta = build_metadata(args.app_name, args.app_version, args.install_dir)
        ti = tarfile.TarInfo("metadata.toml")
        ti.size = len(meta)
        ti.mtime = int(time.time())
        ti.mode = 0o644
        tar.addfile(ti, io.BytesIO(meta))
    tar_data = tar_bytes.getvalue()

    # Compress with LZMA
    lz_data = lzma.compress(tar_data, preset=6)

    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    with open(args.bootstrap_exe, "rb") as f_boot, open(args.output, "wb") as f_out:
        boot = f_boot.read()
        f_out.write(boot)
        magic_offset = f_out.tell()
        f_out.write(MAGIC)
        f_out.write(ALGO)
        f_out.write(lz_data)
        meta_bytes = build_metadata(args.app_name, args.app_version, args.install_dir)
        f_out.write(meta_bytes)
        # trailer: sizes to locate blob
        trailer = TRAILER_STRUCT.pack(len(lz_data), len(meta_bytes), magic_offset)
        f_out.write(trailer)

    print(f"Wrote setup: {args.output}")


if __name__ == "__main__":
    sys.exit(main())
