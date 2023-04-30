#include "redis.hpp"

cpp_redis::client Redis::client_;

cpp_redis::client& Redis::db()
{
    if (!client_.is_connected()) {
        Connect();
    }

    return client_;
}

void Redis::Connect()
{
    auto ip = std::getenv("REDIS_IP");
    auto port = std::stoi(std::getenv("REDIS_PORT"));
    client_.connect(ip, port);
}
