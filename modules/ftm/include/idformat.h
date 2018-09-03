#ifndef _ID_FORMAT_H_
#define _ID_FORMAT_H_

#include <string>
#include <inttypes.h>
#include "graph.h"



#define ID_FID_BITS             4
#define ID_FID_POS              60
#define ID_FID_MSK              ((1 << ID_FID_BITS ) - 1)

typedef struct {
  const std::string& s;
  const uint8_t pos;
  const uint8_t bits;
} propFormat;

typedef std::vector<propFormat> vPf;
extern std::vector<vPf> idFormats;


#endif