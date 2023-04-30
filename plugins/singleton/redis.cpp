#include <sampgdk/core.h>

#include "redis.hpp"

cpp_redis::client& Redis::db()
{
    if (!client_.is_connected()) {
        Connect();
    }

    return client_;
}

void Redis::Connect()
{
    auto ip = "redis";///< @todo std::getenv("REDIS_IP");
    auto port = 6379;///< @todo std::stoi(std::getenv("REDIS_PORT"));
    sampgdk::logprintf("[singleton:redis] connecting to redis %s:%d...", ip, port);
    client_.connect(ip, port);
}
