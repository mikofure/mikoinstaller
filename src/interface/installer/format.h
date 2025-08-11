#pragma once

// Packaging format constants shared between Python packer and C++ extractor
// Layout in output EXE:
// [bootstrap exe bytes]
// magic: "MIKOSETUP\0" (10 bytes)
// algo: 4 ASCII bytes (e.g., "LZMA" or "NONE")
// blob: compressed or raw TAR bytes (size = blob_size from trailer)
// meta: TOML bytes (size = meta_size from trailer)
// trailer: 3x uint64 little-endian: blob_size, meta_size, magic_offset

static const char MIKO_MAGIC[10] = {'M','I','K','O','S','E','T','U','P','\0'};
static const int MIKO_MAGIC_LEN = 10;
static const int MIKO_ALGO_LEN = 4; // "LZMA" or "NONE"
