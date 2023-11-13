/*************************************************************

	This file is the main entry point for the .dll that will
	be injected into the gta_sa.exe file. It must hijack the
	objects within the game's source code and manipulate the
	memory according to the server's sync.

 *************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <Windows.h>

#include "../Player/Player.h"

#define DEBUG_MODE true

/**
* @dev The main thread's function. This is the core of the client, here we will handle the data received/sent from/to the server. 
**/
ULONG mainThread(void* hModule) 
{
    #if DEBUG_MODE == true 
        AllocConsole(); 
        freopen("CONOUT$", "w", stdout);
        std::cout << "Console allocated.\n";
    #endif

    /* Handle DLL code. */
    Player* player = (Player*)(0xB6F5F0); // Local Player

    while (!GetAsyncKeyState(VK_END))
    {
        /* GAME SETTINGS */
        *(float*)(0x8D2530) = 0; // disable pedestrians
        *(float*)(0x8A5B20) = 0; // disable vehicles
        *(BYTE*)(0x96C009) = 1; // free PayNSpray
        *(BYTE*)(0xB7CEE4) = 1; // infinite run
        //memset((void*)0xA7A6A0, 0, 69000); // disable mission block

        if (GetAsyncKeyState(VK_F5) & 1) {
            player->sendTextBox(player->getLastTypedCharacters());
            player->resetLastTypedCharacters();
        }
        if (GetAsyncKeyState(VK_F6) & 1) {
            player->resetLastTypedCharacters();
        }
        if (GetAsyncKeyState(VK_F7) & 1) {

        }

        Sleep(10);
    }
    /* Handle DLL closure. Everything from the DLL must be deallocated. */
    #if DEBUG_MODE == true
        std::cout << "Console freed.\n";
        FreeConsole();
    #endif 
    FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    return 0ul;
}  

/**
* @dev DLL's main function. Creates the mainThread which continues the code's execution.
**/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainThread, hModule, 0, 0);
    }
    return TRUE;
}

