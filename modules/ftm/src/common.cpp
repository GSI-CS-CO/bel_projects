#include "common.h"

void hexDump (const char *desc, const char* addr, int len) {
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  printf ("\n");
}

void hexDump (const char *desc, vBuf vb) { hexDump(desc, (const char*)&vb[0], vb.size()); }

vBl leadingOne(size_t length) {vBl ret(length, false); *ret.begin() = true; return ret;}

std::string fixArchiveVersion(const std::string& s) {
    //hack to ensure correct boost textarchive version
    //not nice, but I'm fed up to here with the crappy boost archive documentation
    const std::string tag = "serialization::archive ";
    const std::string myVer = "10"; // Boost Version 1.53 Archiver Version is 10
    std::string sRet = s;
    size_t pos = sRet.find(tag, 0) + tag.length();
    sRet.replace(pos, myVer.length(), myVer);

    return sRet;

}