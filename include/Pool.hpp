#pragma once

#include <list>
#include <exception>
#include <pthread.h>

template<typename T>
class Pool {

public:
    typedef typename std::list<T>::iterator iterator;
    typedef typename std::list<T>::const_iterator const_iterator;
    typedef typename std::list<T>::reverse_iterator reverse_iterator;
    typedef typename std::list<T>::const_reverse_iterator const_reverse_iterator;

    typedef typename std::list<T>::pointer           pointer;
    typedef typename std::list<T>::reference         reference;
    typedef typename std::list<T>::const_pointer     const_pointer;
    typedef typename std::list<T>::const_reference   const_reference;

    typedef typename std::list<T>::size_type         size_type;
    typedef typename std::list<T>::value_type        value_type;
    typedef typename std::list<T>::allocator_type    allocator_type;
    typedef typename std::list<T>::difference_type   difference_type;

    class Empty : public std::out_of_range {
    private:
        const char *_msg;
    
    public:
        Empty(const char *msg) : std::out_of_range(msg), _msg(msg) {}
        virtual ~Empty(void) throw() {}
    
        virtual const char * what() const throw() {
            return _msg; 
        }
    };

private:
    pthread_mutex_t _locker;
    std::list<T>    _pool;
    bool            _locked;

public:
    Pool(void) : _locked(false) {
        pthread_mutex_init(&_locker, NULL); 
    }

    ~Pool(void) {
        pthread_mutex_destroy(&_locker);
    }

    iterator begin() { return _pool.begin(); }
    const_iterator cbegin() const { return _pool.begin(); }
    iterator end() { return _pool.end(); }
    const_iterator cend() const { return _pool.end(); }

    reverse_iterator rbegin() { return _pool.rbegin(); }
    const_reverse_iterator crbegin() const { return _pool.rbegin(); }
    reverse_iterator rend() { return _pool.rend(); }
    const_reverse_iterator crend() const { return _pool.rend(); }

    void lock(void) {
        pthread_mutex_lock(&_locker);
        _locked = true;
    }

    void unlock(void) {
        _locked = false;
        pthread_mutex_unlock(&_locker);
    }

    void throw_if_empty(void) {
        if (_pool.begin() == _pool.end()) {
            if (!_locked) {
                pthread_mutex_unlock(&_locker);
            }
            throw Empty("Container is empty");
        }
    }

    reference pop_front(void) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        throw_if_empty();
        reference ref = _pool.front();
        _pool.pop_front();

        if (!_locked) {    
            pthread_mutex_unlock(&_locker);
        }

        return ref;
    }

    reference pop_back(void) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        throw_if_empty();
        reference ref = _pool.back();
        _pool.pop_back();

        if (!_locked) {    
            pthread_mutex_unlock(&_locker);
        }

        return ref;
    }

    void push_front(const_reference ref) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        _pool.push_front(ref);

        if (!_locked) {    
            pthread_mutex_unlock(&_locker);
        }
    }

    void push_back(const_reference ref) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        _pool.push_back(ref);

        if (!_locked) {    
            pthread_mutex_unlock(&_locker);
        }
    }

    reference front(void) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        throw_if_empty();
        reference ref = _pool.back();

        if (!_locked) {    
            pthread_mutex_unlock(&_locker);
        }

        return ref;
    }

    reference back(void) {

        if (!_locked) {    
            pthread_mutex_lock(&_locker);
        }

        throw_if_empty();
        reference ref = _pool.back();

        if (!_locked) {
            pthread_mutex_unlock(&_locker);
        }

        return ref;
    }

    size_type size(void) {

        if (!_locked) {
            pthread_mutex_lock(&_locker);
        }

        size_type s = _pool.size();

        if (!_locked) {
            pthread_mutex_unlock(&_locker);
        }

        return s;
    }

    bool empty(void) {

        if (!_locked) {
            pthread_mutex_lock(&_locker);
        }

        bool empty = _pool.empty();

        if (!_locked) {
            pthread_mutex_unlock(&_locker);
        }

        return empty;
    }

};
