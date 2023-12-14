#include "log.h"

log_level_t GLOBAL_LEVEL = ERROR;

const char* const log_lvl_str[] = {
	"NOTHING:",
    "CRITICAL:",
    "ERROR:",
    "WARNING:",
    "INFO:",
    "VERBOSE:",
    "DEBUG LVL 0:",
    "DEBUG LVL 1:",
    "DEBUG LVL 2:",
    "DEBUG LVL 3:",
};
