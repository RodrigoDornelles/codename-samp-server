#ifndef MANAGER_MAP_H
#define MANAGER_MAP_H

#include "limits.h"

namespace manager
{

class Map {
    public:
        static void init(void);
        static void OnPlayerConnect(uint16_t playerid);
        static void AddPlayerObjects(uint16_t playerid, uint16_t world_id);
        static void ClearPlayerObjects(uint16_t playerid);
    
    private:
        inline static uint16_t player_objects[MAX_PLAYERS];
        inline static uint16_t player_object[MAX_PLAYERS][MAX_OBJECTS];
};

}

#endif
