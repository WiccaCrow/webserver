#pragma once

#include <pthread.h>
#include <list>
#include <map>
#include <iostream>
#include <unistd.h>

#include "Response.hpp"


class PoolController {
private:
    static std::list<HTTP::Request *> poolReq;
    static std::list<HTTP::Response *> poolRes;
    static pthread_mutex_t _lock_req;
    static pthread_mutex_t _lock_res;
    
public:
    int id;
    
    PoolController(void);
    ~PoolController(void);

    static void mutexInit(void);
    static void mutexDestroy(void);

    HTTP::Request *getRequest(void);
    HTTP::Response *getResponse(void);
    void putRequest(HTTP::Request *req);
    void putResponse(HTTP::Response *res);

};
