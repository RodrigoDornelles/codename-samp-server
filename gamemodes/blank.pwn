native print(const string[]);
forward OnPlayerConnect(playerid);
forward OnPlayerCommandText(playerid, cmdtext[]);
forward OnPlayerSpawn(playerid);
forward OnGameModeInit();

main()
{
	print("\ngamodemode load: blank.pwn");
}

public OnPlayerConnect(playerid)
{
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	return 1;
}

public OnPlayerSpawn(playerid)
{
	return 1;
}

public OnGameModeInit()
{
	return 1;
}
