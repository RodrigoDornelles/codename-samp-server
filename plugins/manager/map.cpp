#include <regex>
#include <string>
#include <fstream>

#include <sampgdk/a_players.h>
#include <sampgdk/a_objects.h>
#include <sampgdk/core.h>

#include "singleton/redis.hpp"
#include "manager/map.hpp"

namespace manager
{

void Map::init(void)
{
    uint64_t total_obj = 0;
    uint64_t total_del = 0;

    Redis::db().del({"manager:map:obj:1", "manager:map:del:1"});
    Redis::db().sync_commit();

    std::ifstream file("scriptfiles/map-1.txt");
    if (!file.is_open()) {
        sampgdk::logprintf("[manager:map] ERROR: failed to open file.");
        return;
    }

    std::regex pattern_obj(R"(^CreateObject\((\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");
    std::regex pattern_del(R"(^RemoveBuildingForPlayer\(\w+,\s*(\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");
    std::smatch match;
    std::string line;

    while (std::getline(file, line)) {
        if (std::regex_match(line, match, pattern_obj)) {
            if (match.size() == 8) {
                total_obj += 1;
                std::string newLine = "";
                newLine += match[1].str() + ",";
                newLine += match[2].str() + ",";
                newLine += match[3].str() + ",";
                newLine += match[4].str() + ",";
                newLine += match[5].str() + ",";
                newLine += match[6].str() + ",";
                newLine += match[7].str();
                Redis::db().lpush("manager:map:obj:1", {newLine});
            }
        }
        if (std::regex_match(line, match, pattern_del)) {
            if (match.size() == 6) {
                total_del += 1;
                std::string newLine = "";
                newLine += match[1].str() + ",";
                newLine += match[2].str() + ",";
                newLine += match[3].str() + ",";
                newLine += match[4].str() + ",";
                newLine += match[5].str();
                Redis::db().lpush("manager:map:del:1", {newLine});
            }
        }
    }
    sampgdk::logprintf("[manager:map] uploading to database... [obj: %llu del:%llu]", total_obj, total_del);
    Redis::db().commit();
}

/**
 * @todo move to utils/
 */
std::vector<std::string> splitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void Map::OnPlayerConnect(uint16_t playerid)
{
    Redis::db().lrange("manager:map:del:1", 0, -1, [playerid](cpp_redis::reply& reply) {
        if (reply.is_array()) {
            for (const auto& element : reply.as_array()) {
                if (element.is_string()) {
                    std::string line = element.as_string();

                    auto tokens = splitString(line, ',');

                    if (tokens.size() == 5) {
                        RemoveBuildingForPlayer(playerid, std::stoi(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]));
                    }
                }
            }
        }
    });
    Redis::db().commit();
}

void Map::OnPlayerSpawn(uint16_t playerid)
{
    /** remove old objects */
    for (uint16_t object_id = 0; object_id < player_objects[playerid]; object_id++)
    {
        DestroyPlayerObject(playerid, player_object[playerid][object_id]);
    }
    /** add new objects */
    Redis::db().lrange("manager:map:obj:1", 0, -1, [playerid](cpp_redis::reply& reply) {
        if (reply.is_array()) {
            for (const auto& element : reply.as_array()) {
                if (element.is_string()) {
                    std::string line = element.as_string();

                    auto tokens = splitString(line, ',');

                    if (tokens.size() == 7) {
                        auto object_id = player_objects[playerid];
                        player_object[playerid][object_id] = CreatePlayerObject(playerid, std::stoi(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]), std::stof(tokens[5]), std::stof(tokens[6]), 300.0);
                        player_objects[playerid]++;
                    }
                }
            }
        }
    });
    Redis::db().commit();
}

}
