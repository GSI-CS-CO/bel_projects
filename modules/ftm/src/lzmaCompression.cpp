#include "lzmaCompression.h"




std::string errCode(int status) {
  std::string errCode;
  switch (status) {
    case  SZ_OK:                errCode = "OK"; break;
    case  SZ_ERROR_DATA:        errCode = "DATA"; break;
    case  SZ_ERROR_MEM:         errCode = "MEM"; break;
    case  SZ_ERROR_CRC:         errCode = "CRC"; break;
    case  SZ_ERROR_UNSUPPORTED: errCode = "UNSUPPORTED"; break;
    case  SZ_ERROR_PARAM:       errCode = "PARAM"; break;
    case  SZ_ERROR_INPUT_EOF:   errCode = "INPUT_EOF"; break;
    case  SZ_ERROR_OUTPUT_EOF:  errCode = "OUTPUT_EOF"; break;
    case  SZ_ERROR_READ:        errCode = "READ"; break;
    case  SZ_ERROR_WRITE:       errCode = "WRITE"; break;
    case  SZ_ERROR_PROGRESS:    errCode = "PROGRESS"; break;
    case  SZ_ERROR_FAIL:        errCode = "FAIL"; break;
    case  SZ_ERROR_THREAD:      errCode = "THREAD"; break;
    case  SZ_ERROR_ARCHIVE:     errCode = "ARCHIVE"; break;
    case  SZ_ERROR_NO_ARCHIVE:  errCode = "NO_ARCHIVE"; break;
    default:                    errCode = "!UNDEFINED!";
  }
  return errCode;
}

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

vBuf lzmaCompress(const vBuf& input) {
  vBuf result;
  int lzmaStatus;

  // set up properties
  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  if (input.size() >= (1 << 20))
    props.dictSize = 1 << 20; // 1mb dictionary
  else
    props.dictSize = input.size(); // smaller dictionary = faster!
  props.fb = 40;

  // prepare space for the encoded properties
  SizeT propsSize = 5;
  uint8_t propsEncoded[5];

  // calculate necessary space for the compression output. this is way more than necessary in most cases...
  // but better safe than sorry. (a smarter implementation would use a growing buffer,
  // but this requires a bunch of fuckery that is out of scope for this simple case)

  // Set size to input * 1.5, but at least to 1024
  SizeT outputSize64 = input.size() * 1.5 >= 1024 ? input.size() * 1.5 : 1024;

  //allocate buffer and call lzma library compression routine
  vBuf output(outputSize64);
  lzmaStatus = LzmaEncode( output.data(), &outputSize64, input.data(), input.size(), &props, propsEncoded, &propsSize, 0, NULL, &_allocFuncs, &_allocFuncs);

  //if it took less size than allocated (very likely), resize. otherwise we'd be copying nodes with empty stuff to the FW, wasting memory in the embedded system
  output.resize(outputSize64);

  if (lzmaStatus == SZ_OK) {
    // tricky: we have to generate the LZMA header
    // 5 bytes properties + 8 byte uncompressed size
    result.insert(result.end(), propsEncoded, propsEncoded + 5);
    for (int i = 0; i < 8; i++)
      result.push_back( (input.size() >> (i * 8)) & 0xFF ) ;
    result.insert(result.end(), output.begin(), output.end());
  } else {
    throw std::runtime_error("Compression: LZMA reported ERROR " + errCode(lzmaStatus) + " (" + std::to_string(lzmaStatus) + ")\n");
  }

  return result;
}


vBuf lzmaDecompress(const vBuf& input) {
  vBuf result;
  if (input.size() < 13) return result; // too small for valid header

  // extract the decompressed size from the header
  UInt64 size = 0;
  for (int i = 0; i < 8; i++)
    size |= (input[5 + i] << (i * 8));

  if (size <= (256 * 1024 * 1024)) {
    int status;
    ELzmaStatus lzmaStatus;
    SizeT procOutSize = size, procInSize = input.size() - 13;

    //got input and output sizes, allocate the output space in result vector
    //(use resize, not reserve. we're writing C-style to result.data directly, it can't count how much we've written)
    result.resize(procOutSize);

    //call lzma lib decompression
    status = LzmaDecode(result.data(), &procOutSize, (uint8_t*)&input[13], &procInSize, input.data(), 5, LZMA_FINISH_END, &lzmaStatus, &_allocFuncs);
    if (status == SZ_OK) {
      if (procOutSize == size) {
        return result;
      } else throw std::runtime_error("Decompression: LZMA output size ( " + std::to_string(procOutSize) + ") does not match size value in header (" + std::to_string(size) + "\n");
    } else throw std::runtime_error("Decompression: LZMA reported ERROR " + errCode(status) + " (" + std::to_string(status) + ")\n");
  } else throw std::runtime_error("Decompression: LZMA output size too high (" + std::to_string(size) + ", allowed is " + std::to_string(256 * 1024 * 1024) + "\n");

  return result;
}
