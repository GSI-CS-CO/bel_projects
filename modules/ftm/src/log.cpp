#include "log.h"

log_level_t GLOBAL_LOG_LEVEL = ERROR;
bool OUTPUT_GLOBAL_LOG_LEVEL = false;


const char* const log_lvl_str[] = {
    "NONE",
    "ALWAYS",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "INFO",
    "VERBOSE",
    "DEBUG LVL 0",
    "DEBUG LVL 1",
    "DEBUG LVL 2",
    "DEBUG LVL 3",
};

/* Examples:
std::string:
    log<INFO>("Node %1% has an...") % name.c_str();
signed/unsigned
    log<WARNING>("Number %1% is greater...") % counter;
hex n nibbles
    log<INFO>("Address is 0x%1$#08x...") % adr;
*/
