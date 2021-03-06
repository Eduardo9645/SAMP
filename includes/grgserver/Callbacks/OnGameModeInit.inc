/**
 * <summary>
 *	This callback gets called once the gamemode starts.
 * </summary>
 *
 * <remarks>
 *	All initialization stuff is done in this callback.
 * </remarks>
 */

public OnGameModeInit()
{
	print("* Loading Gamemode...");

	printf("    SVN Revision: %d", COMPILER_SVN_REVISION);
	printf("    Compile date: %s %s", COMPILER_DATE, COMPILER_TIME);
	printf("    Last committer: %s", COMPILER_SVN_LASTCOMMITTER);

	if (fexist(FILE_LASTCHANGES))
	{
		new logLine[256];
		new File:changesFile = fopen(FILE_LASTCHANGES, io_read);
		fread(changesFile, logLine);// Skip "------------------------------------------------------------------------"
		fread(changesFile, logLine);// Skip "rXXX | user | yyyy-mm-dd hh:ii:ss +xxxx (ddd, dd mmm yyyy) | x line(s)"
		fread(changesFile, logLine);// Skip ""
		print("    SVN Changelog:");
		while (fread(changesFile, logLine))
		{
			RemoveNewLine(logLine);
			if (strlen(logLine) && strcmp(logLine, "------------------------------------------------------------------------"))
			{
				printf("    > %s", logLine);
			}
		}
		fclose(changesFile);
	}

	print("* Defining variables...");
	maxPlayers = GetServerVarAsInt("maxplayers");

	print("* Checking SVN stuff...");
	new File:svnRevisionFile;
	new oldSvnRevision[16];
	if (fexist(FILE_RUNNINGREVISION))
	{
		svnRevisionFile = fopen(FILE_RUNNINGREVISION, io_read);
		if (svnRevisionFile)
		{
			fread(svnRevisionFile, oldSvnRevision, sizeof(oldSvnRevision));
			fclose(svnRevisionFile);
		}
	}
	if (strval(oldSvnRevision) != COMPILER_SVN_REVISION)
	{
		svnRevisionFile = fopen(FILE_RUNNINGREVISION, io_write);
		if (svnRevisionFile)
		{
			fwrite(svnRevisionFile, val2str(COMPILER_SVN_REVISION));
			fclose(svnRevisionFile);
		}
	}

	print("* Initializing MySQL connection...");
	new MySQL:mysqlConnection = mysql_init(LOG_ONLY_ERRORS, true);
	if (!mysql_connect(MYSQL_HOST, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE, mysqlConnection, true))
	{
		print("MySQL connection failed!");
		return false;
	}

	print("* Loading object models...");
	if (fexist(FILE_OBJECTMODELS))
	{
		new File:objectModelsFile = fopen(FILE_OBJECTMODELS, io_read);
		if (objectModelsFile)
		{
			new objectModelsCount;
			new line[STRINGLENGTH_OBJECTMODELNAME + 10];
			while (fread(objectModelsFile, line))
			{
				new modelID;
				new name[STRINGLENGTH_OBJECTMODELNAME];
				if (sscanf(line, "p<\t>ds[" #STRINGLENGTH_OBJECTMODELNAME "]", modelID, name))
				{
					print("  File format error!");
				}
				else
				{
					format(objectModels[modelID], STRINGLENGTH_OBJECTMODELNAME, name);
					objectModelsCount++;
				}
			}
			fclose(objectModelsFile);
			printf("  %d object models loaded", objectModelsCount);
		}
	}
	else
	{
		print("  Object models file does not exist!");
	}

	print("* Loading weapons...");
	if (fexist(FILE_WEAPONS))
	{
		new File:weaponFile = fopen(FILE_WEAPONS, io_read);
		if (weaponFile)
		{
			new weaponCount;
			new line[256];
			while (fread(weaponFile, line))
			{
				new weaponID;
				new ammo;
				new name[STRINGLENGTH_WEAPONNAME];
				if (sscanf(line, "p<\t>ds[" #STRINGLENGTH_WEAPONNAME "]d", weaponID, name, ammo))
				{
					print("  File format error!");
				}
				else
				{
					if (weaponID >= 0 && weaponID <= 46)
					{
						format(weapons[weaponID][WEAPON_NAME], STRINGLENGTH_WEAPONNAME, name);
						weapons[weaponID][WEAPON_AMMO] = ammo;
						weaponCount++;
					}
				}
			}
			fclose(weaponFile);
			printf("  %d weapons loaded", weaponCount);
		}
	}
	else
	{
		print("  Weapon file does not exist!");
	}

	print("* Loading zones...");
	if (fexist(FILE_ZONES))
	{
		new File:zoneFile = fopen(FILE_ZONES, io_read);
		if (zoneFile)
		{
			new zone;
			new line[256];
			while (fread(zoneFile, line))
			{
				new name[STRINGLENGTH_ZONENAME];
				new Float:startX;
				new Float:startY;
				new Float:startZ;
				new Float:endX;
				new Float:endY;
				new Float:endZ;
				if (sscanf(line, "p<\t>s[" #STRINGLENGTH_ZONENAME "]ffffff", name, startX, startY, startZ, endX, endY, endZ))
				{
					print("  File format error!");
				}
				else
				{
					format(zones[zone][ZONE_NAME], STRINGLENGTH_ZONENAME, name);
					zones[zone][ZONE_STARTX] = startX;
					zones[zone][ZONE_STARTY] = startY;
					zones[zone][ZONE_STARTZ] = startZ;
					zones[zone][ZONE_ENDX] = endX;
					zones[zone][ZONE_ENDY] = endY;
					zones[zone][ZONE_ENDZ] = endZ;
					zone++;
				}
			}
			fclose(zoneFile);
			printf("  %d zones loaded", zone);
		}
	}
	else
	{
		print("  Zone file does not exist!");
	}

	LoadNPCs();
	ReloadLanguageStrings();
	ReloadObjects();
	ReloadVehicleModels();
	ReloadVehicles(true);
	ReloadPickups();
	CreateGlobalTextDraws();

	print("* Adding dummy player classes...");
	AddPlayerClass(0, SPAWN_POSX, SPAWN_POSY, SPAWN_POSZ, SPAWN_ANGLE, 0, 0, 0, 0, 0, 0);

	print("* Doing misc stuff...");
	FormatNew:gameModeTextString[30]("GRG Server r%d", COMPILER_SVN_REVISION);
	SetGameModeText(gameModeTextString);
	EnableStuntBonusForAll(false);
	ShowPlayerMarkers(PLAYER_MARKERS_MODE_OFF);
	LimitPlayerMarkerRadius(6000);
	ManualVehicleEngineAndLights();
	DisableInteriorEnterExits();
	UsePlayerPedAnims();
	Streamer_VisibleItems(STREAMER_TYPE_OBJECT, MAX_OBJECTS);

	print("* Starting timers...");
	StartTimer:EVERYMINUTE("Timer_EveryMinute", true);
	StartTimer:FAST("Timer_Fast", true);
	StartTimer:SLOW("Timer_Slow", true);
	StartTimer:STABLECHECK("Timer_StableCheck", false);

	return true;
}