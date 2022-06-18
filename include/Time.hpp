#pragma once

#include <ctime>
#include <string>

namespace Time {

    const char * const FORMAT_LOC = "%d/%m/%Y %H:%M:%S";
    const char * const FORMAT_GMT = "%a, %-e %b %Y %H:%M:%S GMT";

    std::string time2str(struct tm *t, const char *f);

    std::string local(time_t t = time(0), const char *f = FORMAT_LOC);
    std::string local(const char *f, time_t t = time(0));

    std::string gmt(time_t t = time(0), const char *f = FORMAT_GMT);
    std::string gmt(const char *f, time_t t = time(0));
};