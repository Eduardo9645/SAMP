CMD:changepw(playerid, params[], "Accountpasswort �ndern")
{
	ShowDialog(playerid, DIALOG_CHANGEPASSWORD_STEP1);
	return COMMAND_OK;
}