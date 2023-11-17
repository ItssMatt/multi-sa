/*************************************************************

	The CPlayer class functions' implementations are stored
	in this file.

 *************************************************************/

#include "CPlayer.h"

CPlayer CPlayer::gPlayers[MAX_PLAYERS]; // declaration of the global variable

CPlayer::CPlayer(void) {
    id = 0;
    player_socket = INVALID_SOCKET;
    validPlayer = false;
}

int CPlayer::getAvailablePlayerID(void)
{
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (gPlayers[i].validPlayer)
            continue;
        return i;
    }
    return -1;
}

int CPlayer::getConnectedPlayers(void)
{
    int count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (gPlayers[i].validPlayer)
            count++;
    }
    return count;
}