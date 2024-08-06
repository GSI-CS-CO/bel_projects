#ifndef _LOG_H_
#define _LOG_H_

#include <sstream>
#include <boost/format.hpp>
#include <iostream>
#include <codecvt>

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
    formatted_log_t( log_level_t level, const wchar_t* msg ) : fmt(msg), level(level) {}
    ~formatted_log_t() {
        // GLOBAL_LEVEL is a global variable and could be changed at runtime
        // Any customization could be here
	    std::string s = OUTPUT_GLOBAL_LOG_LEVEL ? std::string(log_lvl_str[level]) + ":" : "";
        if ( level <= GLOBAL_LOG_LEVEL ) {
            if (GLOBAL_LOG_COMPATIBILITY) {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::string narrow_fmt = converter.to_bytes(fmt.str());
                cout << s << " " << narrow_fmt << endl;
            } else wcout << s.c_str() << L" " << fmt << endl;

       }

    }        
    template <typename T> 
    formatted_log_t& operator %(T value) {
        fmt % value;
        return *this;
    }    
protected:
    boost::wformat  fmt;
    log_level_t     level;
};
}//namespace log_impl
// Helper function. Class formatted_log_t will not be used directly.
template <log_level_t level>
log_impl::formatted_log_t log(const wchar_t* msg) {
    return log_impl::formatted_log_t( level, msg );
}

#endif /* _LOG_H_ */
