#include "Time.hpp"

namespace Time {

    std::string 
    time2str(struct tm *t, const char *format) {
        char buff[100];
        strftime(buff, sizeof(buff), format, t);
    
        return buff;
    }

    std::string 
    local(time_t t, const char *format) {
        return time2str(localtime(&t), format);
    }

    std::string 
    local(const char *format, time_t t) {
        return local(t, format);
    }

    std::string
    gmt(time_t t, const char *format) {
        return time2str(gmtime(&t), format);
    }

    std::string
    gmt(const char *format, time_t t) {
        return gmt(t, format);
    }

    // struct tm *
    // gmt(time_t t = time(NULL)) {
    //     return gmtime(&t);
    // }

    // struct tm *
    // local(time_t t = time(NULL)) {
    //     return localtime(&t);
    // }

}
