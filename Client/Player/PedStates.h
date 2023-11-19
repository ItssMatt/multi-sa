/*************************************************************

	Every available PedState. These are used by the
	'OnPlayerStateChange' event.

 *************************************************************/

#ifndef PEDSTATES_H
#define PEDSTATES_H

enum ePedStates {
	PLAYER_STATE_EXITING_VEHICLE = 0,
	PLAYER_STATE_ONFOOT = 1,
	PLAYER_STATE_DRIVING = 50,
	PLAYER_STATE_WASTED = 55,
	PLAYER_STATE_BUSTED = 63
};

char* stateToChar(DWORD state) {
	char* ret = (char*)malloc(50);
	switch (state)
	{
	case PLAYER_STATE_EXITING_VEHICLE:
	{
		strcpy_s(ret, 50, "PLAYER_STATE_EXITING_VEHICLE");
		break;
	}
	case PLAYER_STATE_ONFOOT:
	{
		strcpy_s(ret, 50, "PLAYER_STATE_ONFOOT");
		break;
	}
	case PLAYER_STATE_DRIVING:
	{
		strcpy_s(ret, 50, "PLAYER_STATE_DRIVING");
		break;
	}
	case PLAYER_STATE_WASTED:
	{
		strcpy_s(ret, 50, "PLAYER_STATE_WASTED");
		break;
	}
	case PLAYER_STATE_BUSTED:
	{
		strcpy_s(ret, 50, "PLAYER_STATE_BUSTED");
		break;
	}
	default:
	{
		strcpy_s(ret, 50, "UNKNOWN_STATE");
		break;
	}
	}
	return ret;
}

#endif