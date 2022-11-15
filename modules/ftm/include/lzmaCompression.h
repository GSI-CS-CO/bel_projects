#ifndef _LZMA_COMPRESSION_H_
#define _LZMA_COMPRESSION_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"

vBuf lzmaCompress(const vBuf& input);

vBuf lzmaDecompress(const vBuf& input);

#endif