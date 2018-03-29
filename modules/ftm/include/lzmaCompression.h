#ifndef _LZMA_COMPRESSION_H_
#define _LZMA_COMPRESSION_H_


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"


static void *_lzmaAlloc(ISzAllocPtr, size_t size) {
  return new uint8_t[size];
}
static void _lzmaFree(ISzAllocPtr, void *addr) {
  if (!addr)
    return;

  delete[] reinterpret_cast<uint8_t *>(addr);
}

static ISzAlloc _allocFuncs = {
  _lzmaAlloc, _lzmaFree
};


vBuf lzmaCompress(const vBuf& input);

vBuf lzmaDecompress(const vBuf& input);

#endif