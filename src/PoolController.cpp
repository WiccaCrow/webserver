#include "PoolController.hpp"

std::list<HTTP::Request *> PoolController::poolReq;
std::list<HTTP::Response *> PoolController::poolRes;

pthread_mutex_t PoolController::_lock_req;
pthread_mutex_t PoolController::_lock_res;

PoolController::PoolController(void) {
    Log.debug() << "PoolController created" << std::endl;
}

PoolController::~PoolController(void) {}

void
PoolController::mutexInit(void) {
    pthread_mutex_init(&PoolController::_lock_req, NULL);
    pthread_mutex_init(&PoolController::_lock_res, NULL);
}

void
PoolController::mutexDestroy(void) {
    pthread_mutex_destroy(&PoolController::_lock_req);
    pthread_mutex_destroy(&PoolController::_lock_res);
}

HTTP::Request *
PoolController::getRequest(void) {

    HTTP::Request *req = NULL;
    pthread_mutex_lock(&PoolController::_lock_req);

    if (poolReq.begin() != poolReq.end()) {
        req = poolReq.front();
        poolReq.pop_front();
    }

    pthread_mutex_unlock(&PoolController::_lock_req);
    return req;
}

// If client is going to be disconnected poolreq and poolres should be cleared properly
// All of the processing requests, should be interrupted and terminated

HTTP::Response *
PoolController::getResponse(void) {
    HTTP::Response *res = NULL;

    pthread_mutex_lock(&PoolController::_lock_res);

    if (poolRes.begin() != poolRes.end()) {
        res = poolRes.front();
        poolRes.pop_front();
    }

    pthread_mutex_unlock(&PoolController::_lock_res);
    return res;
}

void
PoolController::putResponse(HTTP::Response *res) {
    pthread_mutex_lock(&PoolController::_lock_res);

    // if (poolRes.size() < poolRes.max_size()) { // Pool limit
        poolRes.push_back(res);
    // }

    pthread_mutex_unlock(&PoolController::_lock_res);
}

void
PoolController::putRequest(HTTP::Request *req) {
    pthread_mutex_lock(&PoolController::_lock_req);

    // if (poolReq.size() < poolReq.max_size()) { // Pool limit
        poolReq.push_back(req);
    // }

    pthread_mutex_unlock(&PoolController::_lock_req);
}
