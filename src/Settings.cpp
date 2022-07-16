#include "Settings.hpp"

Settings::Settings(void) {

    max_wait_conn = 128;
    workers = 3;
    worker_timeout = 10000;
    
    max_requests = 100;
    max_client_timeout = 100;
    max_gateway_timeout = 25;
    session_lifetime = 86400; // 1 Day
    
    max_uri_length = 1024;
    max_header_field_length = 2048;

    blind_proxy = false;
    
    chunk_size = 40 * MiB;
    max_reg_file_size = 4 * MiB;
    max_range_size = 2 * MiB;

}

Settings::~Settings(void) {}