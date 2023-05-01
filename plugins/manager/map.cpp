#include <regex>
#include <string>
#include <fstream>
#include <filesystem>

#include <sampgdk/a_players.h>
#include <sampgdk/a_objects.h>
#include <sampgdk/core.h>

#include "singleton/redis.hpp"
#include "manager/map.hpp"

namespace manager
{

void Map::init(void)
{
    static const std::regex pattern_file(R"(^map-([1-9]\d*).txt$)");
    static const std::regex pattern_obj(R"(^CreateObject\((\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");
    static const std::regex pattern_del(R"(^RemoveBuildingForPlayer\(\w+,\s*(\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");

    Redis::db().keys("manager:map:*", [](cpp_redis::reply& reply) {
        std::vector<std::string> keysToDelete;
        if (reply.is_array()) {
            for (const auto& element : reply.as_array()) {
                keysToDelete.emplace_back(element.as_string());
            }
        }

        Redis::db().del(keysToDelete, [](cpp_redis::reply& reply){
            for (const auto& entry : std::filesystem::directory_iterator("scriptfiles")) {
                std::vector<std::string> list_obj;
                std::vector<std::string> list_del;
                std::smatch match;
                std::string line;
                std::string filename = entry.path().filename().string();
                if (std::filesystem::is_regular_file(entry) && std::regex_match(filename, match, pattern_file)) {
                    std::ifstream file("scriptfiles/" + filename);
                    std::string map_id = match[1].str();
                    if (!file.is_open()) {
                        sampgdk::logprintf("[manager:map] ERROR: failed to open file '%s'", filename.c_str());
                        return;
                    }
                    while (std::getline(file, line)) {
                        if (std::regex_match(line, match, pattern_obj)) {
                            if (match.size() == 8) {
                                std::string newLine = "";
                                newLine += match[1].str() + ",";
                                newLine += match[2].str() + ",";
                                newLine += match[3].str() + ",";
                                newLine += match[4].str() + ",";
                                newLine += match[5].str() + ",";
                                newLine += match[6].str() + ",";
                                newLine += match[7].str();
                                list_obj.push_back(newLine);
                            }
                        }
                        if (std::regex_match(line, match, pattern_del)) {
                            if (match.size() == 6) {
                                std::string newLine = "";
                                newLine += match[1].str() + ",";
                                newLine += match[2].str() + ",";
                                newLine += match[3].str() + ",";
                                newLine += match[4].str() + ",";
                                newLine += match[5].str();
                                list_del.push_back(newLine);
                            }
                        }
                    }
                    Redis::db().lpush("manager:map:obj:" + map_id, list_obj, [map_id](cpp_redis::reply& reply){
                        if (reply.is_integer()) {
                            sampgdk::logprintf("[manager:map] uploaded world_id %s with %llu objects to cache.", map_id.c_str(), reply.as_integer());
                        }
                    }).commit();
                    Redis::db().lpush("manager:map:del:" + map_id, list_del, [map_id](cpp_redis::reply& reply){
                        if (reply.is_integer()) {
                            sampgdk::logprintf("[manager:map] uploaded world_id %s with %llu removed builds to cache.", map_id.c_str(), reply.as_integer());
                        }
                    }).commit();
                }
            }
        }).commit();
    }).commit();
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
    }).commit();
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

                    if (tokens.size() == 6) {
                        auto object_id = player_objects[playerid];
                        player_object[playerid][object_id] = CreatePlayerObject(playerid, std::stoi(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]), std::stof(tokens[5]), std::stof(tokens[6]), 300.0);
                        player_objects[playerid]++;
                    }
                }
            }
        }
    }).commit();
}

}
