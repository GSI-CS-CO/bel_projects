#include "log.h"

log_level_t GLOBAL_LOG_LEVEL = ERROR;
bool OUTPUT_GLOBAL_LOG_LEVEL = false;


const char* const log_lvl_str[] = {
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
    log<INFO>(L"Node %1% has an...") % name.c_str();
signed/unsigned
    log<WARNING>(L"Number %1% is greater...") % counter;
hex n nibbles
    log<INFO>(L"Address is 0x%1$#08x...") % adr;
*/
