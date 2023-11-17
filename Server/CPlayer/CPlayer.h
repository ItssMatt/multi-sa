/*************************************************************

	The CPlayer class. These are all the informations
	regarding every player connected on the server, including
	socket, id, health, etc.

 *************************************************************/

#ifndef CPLAYER_H
#define CPLAYER_H

#include <winsock2.h>

#include "../../Client/CVector/CVector.h"

#define MAX_PLAYERS 10

class CPlayer {
public:
    USHORT id;
    SOCKET player_socket;
    bool validPlayer;

    // CONSTRUCTORS
    CPlayer(void);

    static CPlayer gPlayers[MAX_PLAYERS]; // GLOBAL VAR STORING EVERY PLAYER

    // FUNCTIONS THAT CAN BE CALLED OUTSIDE THIS CLASS
    static int getAvailablePlayerID(void);
    static int getConnectedPlayers(void);
};

#endif