#include "Time.hpp"

namespace Time {

    bool operator>(struct tm &tm1, struct tm &tm2) {
        time_t t1 = mktime(&tm1);
        time_t t2 = mktime(&tm2);

        double diff = difftime(t1, t2);
        return diff > 0.0;
    }

    std::string 
    time2str(struct tm *t, const char *format) {
        char buff[100];
        strftime(buff, sizeof(buff), format, t);
    
        return buff;
    }

    bool
    str2time(const std::string &s, struct tm *t, const char *format) {
        return (strptime(s.c_str(), format, t) != NULL);
    }

    std::string
    local(time_t t, const char *format) {
        return time2str(localtime(t), format);
    }

    std::string 
    local(const char *format, time_t t) {
        return local(t, format);
    }

    std::string
    gmt(time_t t, const char *format) {
        return time2str(gmtime(t), format);
    }

    std::string
    gmt(const char *format, time_t t) {
        return gmt(t, format);
    }

    bool
    gmt(const std::string &s, struct tm *t, const char *format) {
        return str2time(s, t, format);
    }

    struct tm *
    gmtime(time_t t) {
        return std::gmtime(&t);
    }

    struct tm *
    localtime(time_t t) {
        return std::localtime(&t);
    }

}
