#pragma once

#define HTML_BEG "<!DOCTYPE html><html>"
#define HTML_END "</html>"

#define HEAD_BEG "<head>"
#define HEAD_END "</head>"

#define TITLE_BEG "<title>"
#define TITLE_END "</title>"

#define BODY_BEG "<body>"
#define BODY_END "</body>"

#define H1_CENTER_BEG "<center><h1>"
#define H1_CENTER_END "</h1></center>"

#define H1_BEG "<h1>"
#define H1_END "</h1>"

#define HR "<hr>"
#define META_UTF8 "<meta charset=\"UTF-8\">"

#define TABLE_BEG "<table>"
#define TABLE_END "</table>"

#define TD_BEG "<td>"
#define TD_END "</td>"

#define TR_BEG "<tr>"
#define TR_END "</tr>"

#define TH_BEG "<th>"
#define TH_END "</th>"

#define B_BEG "<b>"
#define B_END "</b>"

#define DEF_PAGE_BEG HTML_BEG BODY_BEG H1_BEG
#define DEF_PAGE_END H1_END BODY_END HTML_END

#define DEFAULT_CSS \
        "<style>" \
        "* { color: #60A060; font-family: monospace; }" \
        "a { text-decoration: none; color: #303030; }" \
        "body { padding: 20px; }" \
        "tr td:nth-child(3n), th:nth-child(3n) { text-align: right; }" \
        "tr td:nth-child(3n + 1), th:nth-child(3n + 1) { text-align: left; }" \
        "tr td:nth-child(3n + 2), th:nth-child(3n + 2) { text-align: center; }" \
        "th, td { white-space: nowrap; padding: 5px; }" \
        "</style>"