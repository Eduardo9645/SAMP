CMD:repair(playerID, params[], 296)
{
	if (PVar:playerID[JOBID] == JOB_ADAC)
	{
		new targetPlayerID;
		new Float:price;
		if (sscanf(params, "uf", targetPlayerID, price))
		{
			CmdUsageMsg(playerID, "repair", "Playername/ID", LanguageString(playerID, 298));
			return COMMAND_USAGE;
		}
		if (PVar:targetPlayerID[MONEY] >= price)
		{
			if (IsPlayerInAnyVehicle(targetPlayerID))
			{
				PVar:targetPlayerID[REPAIR_EXECUTOR] = playerID;
				PVar:targetPlayerID[REPAIR_PRICE] = price;
				ShowDialog(targetPlayerID, DIALOG_REPAIR);
				SendClientMessageEx(playerID, COLOR_INFO, 303);
				return COMMAND_OK;
			}
			else
			{
				SendClientMessageEx(playerID, COLOR_ERROR, 300, targetPlayerID);
				return COMMAND_ERROR;
			}
		}
		else
		{
			SendClientMessageEx(playerID, COLOR_ERROR, 299, targetPlayerID);
			return COMMAND_ERROR;
		}
	}
	return COMMAND_PERMISSION_DENIED;
}