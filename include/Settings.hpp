#pragma once

#include "Globals.hpp"

struct Settings {

    std::size_t max_wait_conn;
    
    std::size_t workers;
    std::time_t worker_timeout;
    
    std::size_t max_requests;
    std::time_t max_client_timeout;
    std::time_t max_gateway_timeout;
    
    std::size_t max_uri_length;
    std::size_t max_header_field_length;
    
    bool blind_proxy;
    
    std::size_t session_lifetime; // ?

    uint64_t chunk_size;
    uint64_t max_reg_file_size;
    uint64_t max_range_size;
};