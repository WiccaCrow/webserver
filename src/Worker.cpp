#include "Worker.hpp"
#include "Server.hpp"

size_t Worker::count = 0;

Worker::Worker(void) : _id(count++) {
    Log.debug() << "Worker:: [" << _id << "] created" << Log.endl;
}

Worker::Worker(const Worker &other) {
    *this = other;
}

Worker &
Worker::operator=(const Worker &other) {
    if (this != &other) {
        _id = other._id;
        _thread = other._thread;
    }
    return *this;
}

Worker::~Worker(void) {
    Log.debug() << "Worker:: [" << _id << "] destroyed" << Log.endl;
}

int
Worker::id(void) const {
    return _id;
}

int 
Worker::create(void) {
    if (pthread_create(&_thread, NULL, _cycle, this)) {
        Log.syserr() << "Server::pthread_create failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

int 
Worker::detach(void) {
    if (pthread_detach(_thread)) {
        Log.syserr() << "Server::pthread_detach failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

int 
Worker::join(void) {
    if (pthread_join(_thread, NULL)) {
        Log.syserr() << "Server::pthread_join failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

static HTTP::Request *
popRequest(void) {
    try {
        HTTP::Request *req = g_server->requests.pop_front();
        return req;

    } catch (Pool<HTTP::Request *>::Empty &e) {            
        return NULL;
    }
}

void *
Worker::_cycle(void *ptr) {
    
    Worker *w = reinterpret_cast<Worker *>(ptr);

    Log.debug() << "Worker:: [" << w->id() << "] started" << Log.endl;
    while (g_server->isWorking()) {

        HTTP::Request *req = popRequest();
        if (req == NULL) {
            usleep(WORKER_TIMEOUT);
            continue ;
        }
    
        HTTP::Response *res = new HTTP::Response(req);
        if (res == NULL) {
            Log.syserr() << "Cannot allocate memory for Response" << Log.endl;
            g_server->requests.push_back(req);
            usleep(WORKER_TIMEOUT);
            continue ;
        }

        const std::string &path = req->getUriRef()._path;
        Log.debug() << "Worker:: [" << w->id() << "] -> " << path << " started" << Log.endl; 
        res->handle();
        Log.debug() << "Worker:: [" << w->id() << "] -> " << path << " finished" << Log.endl;

        g_server->responses.push_back(res);
    }

    Log.debug() << "Worker:: [" << w->id() << "] stopped" << Log.endl;
    return NULL;
}
