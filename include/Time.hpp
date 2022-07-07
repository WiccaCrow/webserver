#pragma once

#include <ctime>
#include <string>

namespace Time {

    const char * const f_loc = "%d/%m/%Y %H:%M:%S";
    const char * const f_gmt = "%a, %e %b %Y %H:%M:%S GMT";

    std::string time2str(struct tm *t, const char *f);

    time_t now(void);

    std::string local(time_t t = now(), const char *f = f_loc);
    std::string local(const char *f, time_t t = now());
    bool local(const std::string &s, struct tm *t, const char *f = f_loc);
    struct tm *localtime(time_t t = now());

    std::string gmt(time_t t = now(), const char *f = f_gmt);
    std::string gmt(const char *f, time_t t = now());
    bool gmt(const std::string &s, struct tm *t, const char *f = f_gmt);
    struct tm *gmtime(time_t t = now());


    bool operator>(struct tm &tm1, struct tm &tm2);
};