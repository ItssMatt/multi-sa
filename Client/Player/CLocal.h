/*************************************************************

	The CLocal class. It holds a pointer to the game ped
	and many variables, including the player's ID on
	the server.

 *************************************************************/

#ifndef CLOCAL_H
#define CLOCAL_H

#include "Player.h"

class CLocal {
public:
	UINT sID; // server id
	Player* ped;
	DWORD newState;
	DWORD oldState;

	CLocal(void);
};
#endif