/*************************************************************

	The Player class. The local player and every player on
	the server is runs by this class.
	The CLocal class. It holds a pointer to the local player
	and many variables.

 *************************************************************/

#ifndef Player_H
#define Player_H

#include <Windows.h>
#include <iostream>
#include <string>

#include "../CVector/CVector.h"

 // (*) - only for local player

class Player {
public:
	CVector3D* getPosition(void);
	float getRotationZAngle(void);
	void logPosition(void);

	DWORD getMoney(void); // (*)
	void setMoney(DWORD value); // (*)
	void setMoneyInstant(DWORD value); // (*) -> updates instantly the displayed money value.

	float getHealth(void);
	float getMaxHealth(void);
	float getArmor(void);

	void setHealth(float value);
	void setMaxHealth(float value);
	void setArmor(float value);

	DWORD getState(void);

	void freeze(void); // de-sync if attempt to freeze an entity locally.
	void unfreeze(void); // de-sync if attempt to unfreeze a frozen entity locally.

	static BYTE getMenuID(void); // (*)
	static bool isPaused(void); // (*)
	CVector2D get2DMapPosition(void); // (*) -> gets the 2d coordinates of the player on the map. only updates when the player enters it from the menu!

	char* getLastTypedCharacters(void); // (*)
	void resetLastTypedCharacters(void); // (*)
	void sendTextBox(char* text); // (*)

	int getWeather(void); // (*)
	void setWeather(int value); // (*)
	BYTE getCurrentDay(void); // (*) -> 1 through 7
	BYTE getCurrentHour(void); // (*)
	BYTE getCurrentMinute(void); // (*)

	void showHUD(void); // (*)
	void hideHUD(void); // (*)
	bool getHUD(void); // (*)
};

class CLocal {
public:
	Player* local_ped;
	DWORD newState;
	DWORD oldState;

	CLocal(void);
};

#endif