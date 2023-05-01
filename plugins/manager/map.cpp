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
    static const std::regex pattern_file_only(R"(^map-only-([1-9]\d*).txt$)");
    static const std::regex pattern_file_except(R"(^map-except-([1-9]\d*).txt$)");
    static const std::regex pattern_obj(R"(^CreateObject\((\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");
    static const std::regex pattern_del(R"(^RemoveBuildingForPlayer\(\w+,\s*(\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+),\s*(-?\d+\.\d+)\);$)");

    Redis::db().lpush("manager:map:obj:0", {"force-delete"});
    Redis::db().sync_commit();

    Redis::db().keys("manager:map:*", [](cpp_redis::reply& reply) {
        std::vector<std::string> keysToDelete;
        if (reply.is_array()) {
            for (const auto& element : reply.as_array()) {
                keysToDelete.emplace_back(element.as_string());
            }
        }

        Redis::db().del(keysToDelete, [keysToDelete](cpp_redis::reply& reply){
            for (const auto& entry : std::filesystem::directory_iterator("scriptfiles")) {
                std::vector<std::string> list_obj;
                std::vector<std::string> list_del;
                std::smatch match;
                std::string line;
                std::string filename = entry.path().filename().string();
                if (std::filesystem::is_regular_file(entry) && std::regex_match(filename, match, pattern_file_only)) {
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
                    if (!list_obj.empty()) {
                        Redis::db().lpush("manager:map:obj:" + map_id, list_obj, [map_id](cpp_redis::reply& reply){
                            if (reply.is_integer()) {
                                sampgdk::logprintf("[manager:map] uploaded world_id %s with %llu objects to cache.", map_id.c_str(), reply.as_integer());
                            }
                        }).commit();
                    }
                    if (!list_del.empty()) {
                        Redis::db().lpush("manager:map:del:" + map_id, list_del, [map_id](cpp_redis::reply& reply){
                            if (reply.is_integer()) {
                                sampgdk::logprintf("[manager:map] uploaded world_id %s with %llu removed builds to cache.", map_id.c_str(), reply.as_integer());
                            }
                        }).commit();
                    }
                }
                if (std::filesystem::is_regular_file(entry) && std::regex_match(filename, match, pattern_file_except)) {
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
                    }
                    if (!list_obj.empty()) {
                        sampgdk::logprintf("[manager:map] uploaded world_id %s with %llu deafult objects to cache.", map_id.c_str(), reply.as_integer());                      
                        for (const auto& key : keysToDelete) {
                            if(key.find("obj", 12) != std::string::npos && ("manager:map:obj:" + map_id) != key){
                                Redis::db().lpush(key, list_obj);
                            }                            
                        }
                        Redis::db().commit();
                    }
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

void Map::ClearPlayerObjects(uint16_t playerid)
{
    while (player_objects[playerid]) {
        player_objects[playerid] -= 1;
        DestroyPlayerObject(playerid, player_object[playerid][player_objects[playerid]]);
    }
}

void Map::AddPlayerObjects(uint16_t playerid, uint16_t world_id)
{
    /** add new objects */
    static const auto func = [playerid](cpp_redis::reply& reply) {
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
    };

    ClearPlayerObjects(playerid);
    if (world_id != 0) {
        Redis::db().lrange("manager:map:obj:0", 0, -1, func).commit();
    }
    Redis::db().lrange("manager:map:obj:" + std::to_string(world_id), 0, -1, func).commit();
}

}
