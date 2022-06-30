#pragma once

#include "Logger.hpp"
#include "Globals.hpp"
#include "Response.hpp"
#include <pthread.h>

class Worker {

public:
    static size_t count;

private:
    int       _id;
    pthread_t _thread;

public:
    Worker(void);
    Worker(const Worker &);
    ~Worker(void);

    Worker &operator=(const Worker &);
    int create(void);
    int detach(void);
    int join(void);
    int id(void) const;

private:
    static void *_cycle(void *ptr);
};
