
#pragma once

#include "predef.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace taboo
{

typedef enum {
    read = 1,
    write = 2,
} Purpose;

extern volatile std::size_t reading, writing;
extern boost::mutex mutex;

class BaseGuard
{
protected:
    void lock()
    {

    }
};

template<Purpose purpose>
class Guard;

template<>
class Guard<read>
{
public:
    Guard() throw()
    {
        if (writing) {
            mutex.lock();
            while (writing) {
                writeDone.wait();
            }
            __sync_add_and_fetch(&reading, 1);
        } else {
            __sync_add_and_fetch(&reading, 1);
        }
    }

    ~Guard() throw()
    {
        if (__sync_sub_and_fetch(&reading, 1) == 0) {
            readDone.notify_one();
        }
    }
};

template<>
class Guard<write>
{
public:
    Guard() throw()
    {
        if (reading) {
            boost::mutex::scoped_lock lock(mutex);
            while (reading) {
                readDone.wait(lock);
            }
        }

        __sync_add_and_fetch(&writing, 1);
    }

    ~Guard() throw()
    {
        if (__sync_sub_and_fetch(&writing, 1) == 0) {
            writeDone.notify_all();
        }
    }
};

}
