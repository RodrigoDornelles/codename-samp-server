#include <stdio.h>
#include <string.h>

#include <sampgdk/a_players.h>
#include <sampgdk/a_objects.h>
#include <sampgdk/a_vehicles.h>
#include <sampgdk/a_samp.h>
#include <sampgdk/core.h>
#include <sampgdk/sdk.h>

void Spawn(int playerid) {
  TogglePlayerSpectating(playerid, 0);
  SetSpawnInfo(playerid, 0, 0, 1786.5126,-1299.9512,120.2656,3.74705, 0, 0, 0, 0, 0, 0 );
  SpawnPlayer(playerid);
}

void SAMPGDK_CALL CameraSpawn0(int timerid, void *params) {
  SetPlayerPos(0, 1451.6907, -807.7687, 84.2744);
  SetPlayerCameraPos(0, 1783.8560,-1295.5787,120.2656);
  SetPlayerCameraLookAt(0, 1451.6907,-807.7687,84.2744, CAMERA_CUT);
}

void SAMPGDK_CALL CameraSpawn1(int timerid, void *params) {
  SetPlayerCameraLookAt(0, 1786.5126,-1299.9512,120.2656, CAMERA_MOVE);
}

void SAMPGDK_CALL CameraSpawn2(int timerid, void *params) {
  Spawn(0);
}

PLUGIN_EXPORT bool PLUGIN_CALL OnGameModeInit() {
  SetGameModeText("Hello, World!");
  AddPlayerClass(0, 1958.3783f, 1343.1572f, 15.3746f, 269.1425f,
                 0, 0, 0, 0, 0, 0);
  /* SetTimer(1000, true, PrintTickCountTimer, 0); */
  return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerConnect(int playerid) {
  SendClientMessage(playerid, 0xFFFFFFFF, "Welcome to the HelloWorld server!");
  TogglePlayerSpectating(playerid, 1);
  SetTimer(100, false, CameraSpawn0, 0);
  SetTimer(1000, false, CameraSpawn1, 0);
  SetTimer(8000, false, CameraSpawn2, 0);
  return true;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerSpawn(int playerid)
{
  SetPlayerHealth(playerid, (float) 0x7f7fffff);
  return 1;
}

PLUGIN_EXPORT bool PLUGIN_CALL OnPlayerCommandText(int playerid,
                                                   const char *cmdtext) {
  if (strcmp(cmdtext, "/hello") == 0) {
    char name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    char message[MAX_CLIENT_MESSAGE];
    sprintf(message, "Hello, %s!", name);
    SendClientMessage(playerid, 0x00FF00FF, message);
    return true;
  }
  if (strcmp(cmdtext, "/jetpack") == 0) {
    SetPlayerSpecialAction(playerid, 2); 
    return true;
  }
  if (strcmp(cmdtext, "/moto") == 0) {
    float x, y, z, a;
    GetPlayerPos(playerid, &x, &y, &z);
    GetPlayerFacingAngle(playerid, &a);
    CreateVehicle(522, x, y, z, a, 1, 1, -1, false);
    SendClientMessage(playerid, 0x00FF00FF, "vrum vrum!");
    return true;
  }
  if (strcmp(cmdtext, "/spawn") == 0) {
    Spawn(playerid);
    return true;
  }
  if (strcmp(cmdtext, "/gmx") == 0) {
    SendClientMessage(playerid, 0x00FF00FF, "GMX!");
    SendRconCommand("gmx");
    return true;
  }
  if (strcmp(cmdtext, "/shutdown") == 0) {
    SendClientMessage(playerid, 0x00FF00FF, "the server is restarting...");
    SendRconCommand("exit");
    return true;
  }
  return false;
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() {
  return sampgdk::Supports() | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData) {
  return sampgdk::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload() {
  sampgdk::Unload();
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick() {
  sampgdk::ProcessTick();
}
