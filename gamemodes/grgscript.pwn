#include <a_samp>
#include <mysql>
#include <irc>
#include <MD5>

#include <grg/constants>
#include <grg/config>

#include <grg/functions>

new ircBot;

main()
{
	print("\n----------------------------------");
	print("GRG Server");
	print("----------------------------------\n");
}

public OnGameModeInit()
{
	mysql_init();
	mysql_connect(MYSQL_HOST, MYSQL_USERNAME, MYSQL_PASSWORD, MYSQL_DATABASE);
	ircBot = IRC_Connect(IRC_HOST, IRC_PORT, IRC_USERNAME, IRC_USERNAME, IRC_USERNAME, IRC_SSL);
	SetGameModeText("GRG Server");
	AddPlayerClass(0, 1958.3783, 1343.1572, 15.3746, 269.1425, 0, 0, 0, 0, 0, 0);
	return true;
}

public OnGameModeExit()
{
	mysql_close();
	IRC_Quit(ircBot, "SAMP-Server IRC chat relay");
	return true;
}

public OnPlayerConnect(playerid)
{
	new query[256];
	new playerName[MAX_PLAYER_NAME];
	SendClientMessage(playerid,COLOR_YELLOW, "Script by Aerox_Tobi and Programie");
	GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
	format(query, sizeof(query), "SELECT * FROM `users` WHERE `Username` = '%s'", mySQLEscapeString(playerName));
	mysql_query(query);
	mysql_store_result();
 	if (strval(getMySQLField("UserID")))
	{
		ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_INPUT, "Login", "Bitte gebe dein Passwort f�r deinen Account ein.", "OK", "Abbrechen");
	}
	else
	{
		ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_INPUT, "Registrieren", "Du bist noch nicht Registriert!\nBitte gebe ein neues Passwort f�r deinen Account ein.", "OK", "Abbrechen");
	}
	mysql_free_result();
	return true;
}

public OnPlayerDisconnect(playerid, reason)
{
	if (GetPVarInt(playerid, "UserID"))
	{
		savePlayer(playerid);
	}
	return true;
}

public OnPlayerRequestClass(playerid, classid)
{
	SetPlayerPos(playerid, 1958.3783, 1343.1572, 15.3746);
	SetPlayerCameraPos(playerid, 1958.3783, 1343.1572, 15.3746);
	SetPlayerCameraLookAt(playerid, 1958.3783, 1343.1572, 15.3746);
	return true;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	new cmd[256];
	new index;
	cmd = strtok(inputtext, index);
	if (strval(inputtext))
	{
		switch (dialogid)
		{
			case DIALOG_LOGIN:
			{
				if (response)
				{
					if (strlen(inputtext))
					{
						new playerName[MAX_PLAYER_NAME];
						new query[256];
						GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
						format(query, sizeof(query), "SELECT * FROM `users` WHERE `Username` = '%s'", mySQLEscapeString(playerName));
						mysql_query(query);
						mysql_store_result();
						if (!strcmp(md5(inputtext), getMySQLField("Password"), true))
						{
							SetPVarInt(playerid, "UserID", strval(getMySQLField("UserID")));
						}
						else
						{
							ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_INPUT, "Login", "Das angegebene Passwort ist falsch!\nBtte versuche es erneut.", "OK", "Abbrechen");
						}
						mysql_free_result();
						if (GetPVarInt(playerid, "UserID"))
						{
							loadPlayer(playerid);
							SpawnPlayer(playerid);
						}
					}
					else
					{
						ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_INPUT, "Login", "Kein Passwort eingegeben!\nBitte versuche es erneut.", "OK", "Abbrechen");
					}
				}
				else
				{
					SendClientMessage(playerid, COLOR_RED, "Du kannst nun das Spiel beenden!");
					Kick(playerid);
				}
				return true;
			}
			case DIALOG_REGISTER:
			{
				if (response)
				{
					if (strlen(inputtext) >= REGISTER_MINPASSWORD && strlen(inputtext) <= REGISTER_MAXPASSWORD)
					{
						new query[1024];
						new playerName[MAX_PLAYER_NAME];
						GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
						format(query, sizeof(query), "INSERT INTO `users` (`Username`, `Password`, `Level`, `AdminLevel`) VALUE ('%s', '%s', '%d', '%d')", mySQLEscapeString(playerName), md5(inputtext), REGISTER_LEVEL, REGISTER_ADMINLEVEL);
						mysql_query(query);
						SetPVarInt(playerid, "UserID", strval(getMySQLValue("users", "UserID", "Username", mySQLEscapeString(playerName))));// TODO: Read UserID from mysql_insert_id() (Not available in the current plugin release)
						SendClientMessage(playerid, COLOR_YELLOW, "Du wurdest erfolgreich registriert und bist nun eingeloggt.");
						SpawnPlayer(playerid);
					}
					else
					{
						new string[256];
						format(string, sizeof(string), "Bitte verwende ein Passwort mit einer L�nge zwischen %d und %d Zeichen!", REGISTER_MINPASSWORD, REGISTER_MAXPASSWORD);
						ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_INPUT, "Registrieren", string, "OK", "Abbrechen");
					}
				}
				else
				{
					SendClientMessage(playerid, COLOR_RED, "Du kannst nun das Spiel beenden!");
					Kick(playerid);
				}
				return true;
			}
		}
	}
	return true;
}

public IRC_OnConnect(botid, ip[], port)
{
	IRC_JoinChannel(botid, IRC_CHANNEL);
}

public IRC_OnJoinChannel(botid, channel[])
{
	IRC_Say(botid, channel, "Hello!");
	IRC_Say(botid, channel, "I'm an IRC chat relay for the SAMP-Server 'GRGServer'.");
}