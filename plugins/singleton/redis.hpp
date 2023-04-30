#ifndef SINGLETON_REDIS_H
#define SINGLETON_REDIS_H

#include <cpp_redis/cpp_redis>

class Redis
{
    public:
        static cpp_redis::client& db();

    private:
        inline static cpp_redis::client client_;
        static void Connect();
};

#endif
