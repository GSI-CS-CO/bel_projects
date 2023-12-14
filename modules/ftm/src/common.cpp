#include "common.h"
#include <ctime>


bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}


std::string hexDump (const char *desc, const char* addr, int len) {
    int i;
    unsigned char buff[17];
    char sbuff[1024];
    const unsigned char *pc = (const unsigned char *)addr;
    std::string ret = "";

    // Output description if given.
    if (desc != NULL)
       sprintf (sbuff, "%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               sprintf (sbuff + strlen(sbuff), "  %s\n", buff);

            // Output the offset.
           sprintf (sbuff + strlen(sbuff), "  %04x ", i);
        }

        // Now the hex code for the specific character.
       sprintf (sbuff + strlen(sbuff), " %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  sprintf (sbuff + strlen(sbuff), "\n");
  return std::string(sbuff);
}

void hexDump (const char *desc, vBuf vb) { hexDump(desc, (const char*)&vb[0], vb.size()); }

vBl leadingOne(size_t length) {vBl ret(length, false); *ret.begin() = true; return ret;}

std::string nsTimeToDate(uint64_t t, bool noSpaces) {
  char date[40];
  uint64_t tAux = t / 1000000000ULL;
  uint64_t tMod = t % 1000000000ULL;
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", gmtime((time_t*)&tAux));
  std::string ret = std::string(date);
  ret += " ";
  ret += std::to_string(tMod);
  ret += "ns\n";
  if (noSpaces) std::replace( ret.begin(), ret.end(), ' ', '_');
  return ret;
}


