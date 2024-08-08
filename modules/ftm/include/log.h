#ifndef _LOG_H_
#define _LOG_H_

#include <sstream>
#include <boost/format.hpp>
#include <iostream>


#define GLOBAL_LOG_COMPATIBILITY 1

enum log_level_t {

    NONE        = 0,
    ALWAYS      = 1,
    CRITICAL    = 2,
    ERROR       = 3,
    WARNING     = 4,
    INFO        = 5,
    VERBOSE     = 6,
    DEBUG_LVL0  = 7,
    DEBUG_LVL1  = 8,
    DEBUG_LVL2  = 9,
    DEBUG_LVL3  = 10,
    DEBUG       = DEBUG_LVL2,
    LOG_MIN     = NONE,
    LOG_MAX     = DEBUG_LVL3,
    LOG_DEFAULT = ERROR,

};


extern const char* const log_lvl_str[];
extern log_level_t GLOBAL_LOG_LEVEL;
extern bool OUTPUT_GLOBAL_LOG_LEVEL;

using namespace std;

namespace log_impl {
class formatted_log_t {


public:
    formatted_log_t( log_level_t level, const char* msg ) : fmt(msg), level(level) {}
    formatted_log_t( log_level_t level, const char* msg, bool useEndl ) : fmt(msg), level(level), useEndl(useEndl) {}
    ~formatted_log_t() {
        // GLOBAL_LEVEL is a global variable and could be changed at runtime
        // Any customization could be here
	    std::string s = OUTPUT_GLOBAL_LOG_LEVEL ? std::string(log_lvl_str[level]) + ": " : "";
        if ( level <= GLOBAL_LOG_LEVEL ) {
          if (useEndl == true) {
            cout << s << fmt << endl;
          } else {
            cout << s << fmt;
          }
        }
    }        
    template <typename T> 
    formatted_log_t& operator %(T value) {
        fmt % value;
        return *this;
    }    
protected:
    boost::format  fmt;
    log_level_t     level;
    bool useEndl = true;
};
}//namespace log_impl
// Helper function. Class formatted_log_t will not be used directly.
template <log_level_t level>
log_impl::formatted_log_t log(const char* msg) {
    return log_impl::formatted_log_t( level, msg );
}

template <log_level_t level>
log_impl::formatted_log_t log(const char* msg, bool useEndl) {
    return log_impl::formatted_log_t( level, msg, useEndl );
}

#endif /* _LOG_H_ */
