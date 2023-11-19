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
    // SERVER LOGIC
    USHORT id; // server-side id
    SOCKET player_socket; // client socket
    bool validPlayer;

    // PLAYER VARIABLES
    float health; // server-side health
    float armor; // server-side armor
    CVector3D position; // server-side position

    // CONSTRUCTORS
    CPlayer(void);

    static CPlayer gPlayers[MAX_PLAYERS]; // Global variable storing every connected player.

    // FUNCTIONS THAT CAN BE CALLED OUTSIDE THIS CLASS
    static int getAvailablePlayerID(void);
    static int getConnectedPlayers(void);
};

#endif