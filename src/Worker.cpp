#include "Worker.hpp"
#include "Server.hpp"

std::size_t Worker::count = 0;

Worker::Worker(void) : _id(count++) {}
Worker::~Worker(void) {}

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

int Worker::id(void) const {
    return _id;
}

int Worker::create(void) {
    if (pthread_create(&_thread, NULL, _cycle, this)) {
        Log.syserr() << "Server::pthread_create failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

int Worker::detach(void) {
    if (pthread_detach(_thread)) {
        Log.syserr() << "Server::pthread_detach failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

int Worker::join(void) {
    if (pthread_join(_thread, NULL)) {
        Log.syserr() << "Server::pthread_join failed for worker " << _id << Log.endl;
        return 0;
    }
    return 1;
}

void *
Worker::_cycle(void *ptr) {
    Worker *w = reinterpret_cast<Worker *>(ptr);

    Log.debug() << "Worker " << w->id() << "::cycle started" << Log.endl;
    while (g_server->working()) {
    
        HTTP::Response *res = g_server->rmFromRespQ();

        if (res == NULL) {
            usleep(g_server->settings.worker_timeout);
            continue;
        }

        // res->getClient()->processing(true);
        const std::string path = res->getRequest()->getUriRef()._path;
        Log.debug() << "Worker " << w->id() << "::cycle: " << path << " started" << Log.endl;
        res->handle();
        Log.debug() << "Worker " << w->id() << "::cycle: " << path << " finished" << Log.endl;
    }

    Log.debug() << "Worker " << w->id() << "::cycle stopped" << Log.endl;
    return NULL;
}
