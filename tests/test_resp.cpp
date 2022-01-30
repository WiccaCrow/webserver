#include <iostream>

std::string _bodyStyle =
    "<!DOCTYPE html>\n"
    "<html lang=en>\n"
    "<meta charset=utf-8>\n"
    "<style>\n"
    "*{margin:0;padding:0}html{font:15px/22px arial,"
    "sans-serif}html{background:#fff;color:#222;"
    "padding:15px}body{margin:7\% auto 0;max-width:"
    "390px;min-height:180px;padding:30px 0 15px}p"
    "{margin:11px 0 22px;overflow:hidden}err_text"
    "{color:#777;text-decoration:none}"
    "@media screen and (max-width:772px){body{background:none;"
    "margin-top:0;max-width:none;padding-right:0}}\n"
    "</style>\n";

int main() {
    _bodyStyle +=
        "<title>Error 505 (HTTP Version Not Supported)</title>\n"
        "<p><b>505.</b> HTTP Version Not Supported."
        "<p><err_text>HTTP version not supported.</err_text>";
    std::cout << _bodyStyle.length() << std::endl;
}
