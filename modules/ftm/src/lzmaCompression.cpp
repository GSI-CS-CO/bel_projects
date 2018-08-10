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


vBuf lzmaCompress(const vBuf& input) {
  // TODO: add decent memory management for compression
  uint8_t b[input.size() * 2];
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

  // allocate some space for the compression output
  // this is way more than necessary in most cases...
  // but better safe than sorry
  //   (a smarter implementation would use a growing buffer,
  //    but this requires a bunch of fuckery that is out of
  ///   scope for this simple example)
  SizeT outputSize64 = input.size() * 1.5;
  if (outputSize64 < 1024)
    outputSize64 = 1024;


  lzmaStatus = LzmaEncode(
    b, &outputSize64, input.data(), input.size(),
    &props, propsEncoded, &propsSize, 0,
    NULL,
    &_allocFuncs, &_allocFuncs);

  //printf("InputSize = %u, Outputsize = %u\n", (unsigned)input.size(), (unsigned)outputSize64);
  vBuf output(b, b + outputSize64);


  if (lzmaStatus == SZ_OK) {
    // tricky: we have to generate the LZMA header
    // 5 bytes properties + 8 byte uncompressed size
    //result.reserve(outputSize64 + 13);
    result.insert(result.end(), propsEncoded, propsEncoded + 5);

    //memcpy(resultData, propsEncoded, 5);
    for (int i = 0; i < 8; i++)
      result.push_back( (input.size() >> (i * 8)) & 0xFF ) ;
    //memcpy(resultData + 13, output.data(), outputSize64);
    result.insert(result.end(), output.begin(), output.end());
  } else {
    throw std::runtime_error("Compression: LZMA reported ERROR " + errCode(lzmaStatus) + " (" + std::to_string(lzmaStatus) + ")\n");
  }
  //printf("ResultSize = %u\n", (unsigned)result.size());
  return result;
}


vBuf lzmaDecompress(const vBuf& input) {
  vBuf result, blob;
  uint8_t b[input.size() * 50];
  int status;
  //printf("Decompress InputSize = %u\n", (unsigned)input.size());

  if (input.size() < 13)
    return result; // invalid header!

  // extract the size from the header
  UInt64 size = 0;
  for (int i = 0; i < 8; i++)
    size |= (input[5 + i] << (i * 8));

  if (size <= (256 * 1024 * 1024)) {
    //blob.reserve(size);

    ELzmaStatus lzmaStatus;
    SizeT procOutSize = size, procInSize = input.size() - 13;
    status = LzmaDecode(b, &procOutSize, (uint8_t*)&input[13], &procInSize, input.data(), 5, LZMA_FINISH_END, &lzmaStatus, &_allocFuncs);

    if (status == SZ_OK && procOutSize == size) {
      //blob.resize(*outputSize);
      result.insert(result.end(), b, b + size);
      return result;
    }
  }

  if (status != SZ_OK) {
    throw std::runtime_error("Decompression: LZMA reported ERROR " + errCode(status) + " (" + std::to_string(status) + ")\n");
  }
  return result;
}
