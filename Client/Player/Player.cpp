/*************************************************************

	The Player class functions' implementations are stored
	in this file.

 *************************************************************/

#include "Player.h"

CVector3D Player::getPosition(void) {
	return *(CVector3D*)(*(DWORD*)(*(DWORD*)this + 0x14) + 0x30);
}

void Player::setPosition(CVector3D position) {
	*(CVector3D*)(*(DWORD*)(*(DWORD*)this + 0x14) + 0x30) = position;
}

float Player::getRotationZAngle(void) {
	return *(float*)(*(DWORD*)this + 0x558);
}

void Player::logPosition(void) {
	std::cout << "X: " << this->getPosition().x << " | Y: " << this->getPosition().y << " | Z: " << this->getPosition().z << std::endl;
}

DWORD Player::getState(void) {
	return *(DWORD*)(*(DWORD*)this + 0x530);
}

DWORD Player::getMoney(void) {
	return (*(DWORD*)0xB7CE50);
}

void Player::setMoney(DWORD value) {
	(*(DWORD*)0xB7CE50) = value;
}

void Player::setMoneyInstant(DWORD value) {
	(*(DWORD*)0xB7CE50) = value;
	(*(DWORD*)(0xB7CE50 + 0x4)) = value;
}

float Player::getHealth(void) { 
	return *(float*)(*(DWORD*)this + 0x540);
}

float Player::getMaxHealth(void) {
	return *(float*)(*(DWORD*)this + 0x544);
}

float Player::getArmor(void) {
	return *(float*)(*(DWORD*)this + 0x548);
}

void Player::setHealth(float value) {
	*(float*)(*(DWORD*)this + 0x540) = value;
}

void Player::setMaxHealth(float value) {
	*(float*)(*(DWORD*)this + 0x544) = value;
}

void Player::setArmor(float value) {
	*(float*)(*(DWORD*)this + 0x548) = value;
}

void Player::freeze(void) {
	*(BYTE*)(*(DWORD*)this + 0x598) = 1;
}

void Player::unfreeze(void) {
	*(BYTE*)(*(DWORD*)this + 0x598) = 0;
}

BYTE Player::getMenuID(void) {
	return *(BYTE*)((DWORD)0xBA6748 + 0x15D);
}

bool Player::isPaused(void) {
	return *(bool*)((DWORD)0xBA6748 + 0x5C);
}

CVector2D Player::get2DMapPosition(void) {
	return *(CVector2D*)((DWORD)0xBA6748 + 0x68);
}

char* Player::getLastTypedCharacters(void) { // characters return mirrored
	char* str = (char*)(0x969110);
	std::string reversed = "";

	for (int i = strlen(str) - 1; i >= 0; i--) {
		reversed += str[i];
	}
	return (char*)reversed.c_str();
}

void Player::resetLastTypedCharacters(void) {
	strcpy_s((char*)(0x969110), 150, "");
}

void Player::sendTextBox(char* text) {
	strcpy_s((char*)(0xBAA7A0), 150, text);
}

int Player::getWeather(void) {
	return *(int*)(0xC81320);
}

void Player::setWeather(int value) {
	*(int*)(0xC81320) = value;
}

BYTE Player::getCurrentDay(void) {
	return *(BYTE*)(0xB7014E);
}

BYTE Player::getCurrentHour(void) {
	return *(BYTE*)(0xB70153);
}

BYTE Player::getCurrentMinute(void) {
	return *(BYTE*)(0xB70152);
}

void Player::showHUD(void) {
	*(BYTE*)(0xBA6769) = 1; // HUD
	*(BYTE*)(0xBA676C) = 0; // RADAR
}

void Player::hideHUD(void) {
	*(BYTE*)(0xBA6769) = 0; // HUD
	*(BYTE*)(0xBA676C) = 2; // RADAR
}

bool Player::getHUD(void) {
	return *(BYTE*)(0xBA6769);
}