/*************************************************************

	This file is the main entry point for the .dll that will
	be injected into the gta_sa.exe file. It must hijack the
	objects within the game's source code and manipulate the
	memory according to the server's sync.

 *************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <cstdlib>
#include <winsock2.h>

#include "../Interface/detours.h"

#pragma comment(lib, "Detours.lib")
#pragma comment(lib, "ws2_32.lib")

#include "../Player/Player.h"

#include "../../Server/RPC/RPC.h"

#include "../Hooks/hooks.hpp"

#define DEBUG_MODE true

SOCKET clientSocket;
CLocal LOCAL_PLAYER;

/**
* @dev The main thread's function. This is the core of the client, here we will handle the data received/sent from/to the server. 
**/
ULONG mainThread(void* hModule) 
{
    /* Handle DLL code. */

    while (!GetAsyncKeyState(VK_END))
    {
        /* GAME SETTINGS */
        *(float*)(0x8D2530) = 0; // disable pedestrians
        *(float*)(0x8A5B20) = 0; // disable vehicles
        *(BYTE*)(0x96C009) = 1; // free PayNSpray
        *(BYTE*)(0xB7CEE4) = 1; // infinite run
        //memset((void*)0xA7A6A0, 0, 69000); // disable mission block

        if (GetAsyncKeyState(VK_F5) & 1) {
            LOCAL_PLAYER.local_ped->sendTextBox(LOCAL_PLAYER.local_ped->getLastTypedCharacters());
            LOCAL_PLAYER.local_ped->resetLastTypedCharacters();
        }
        else if (GetAsyncKeyState(VK_F6) & 1) {
            LOCAL_PLAYER.local_ped->resetLastTypedCharacters();
        }
        else if (GetAsyncKeyState(VK_F7) & 1) {

        }

        // EVENTS

        // OnPlayerTakeDamage
        if (e_onTakeDamage) 
        {
            e_onTakeDamage = false;
            char data[20];
            std::sprintf(data, "%d %f", RPC_ON_PLAYER_TAKE_DAMAGE, LOCAL_PLAYER.local_ped->getHealth());
            int bytesSent = send(clientSocket, data, sizeof(data), 0);
        }

        // OnPlayerStateChange
        if (LOCAL_PLAYER.local_ped != nullptr && !Player::isPaused() && (int)Player::getMenuID() == 41) {
            LOCAL_PLAYER.newState = LOCAL_PLAYER.local_ped->getState();
            if (LOCAL_PLAYER.newState != LOCAL_PLAYER.oldState) {
                if (LOCAL_PLAYER.oldState != -1) {
                    char data[20];
                    std::sprintf(data, "%d %d %d", RPC_ON_PLAYER_STATE_CHANGE, LOCAL_PLAYER.newState, LOCAL_PLAYER.oldState);
                    int bytesSent = send(clientSocket, data, sizeof(data), 0);
                }
                LOCAL_PLAYER.oldState = LOCAL_PLAYER.newState;
            }
        }

        Sleep(10);
    }
    /* Handle DLL closure. Everything from the DLL must be deallocated. */
    // Close the socket and cleanup
    closesocket(clientSocket);
    WSACleanup();
    #if DEBUG_MODE == true
        std::cout << "Console freed.\n";
        FreeConsole();
    #endif 
    FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    return 0ul;
}

/**
* @dev This function is used to handle the server connection + sync/recv. We also run it on a separate thread, because it freezes it!
**/
ULONG tcpThread(void)
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return -1;
    }

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return -1;
    }

    // Server address and port
    // We should pass this information when running the EXE.exe file, through arguments. TODO!
    const char* serverIP = "127.0.0.1";
    const int serverPort = 6666;

    // Fill in the server address details
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(serverIP);
    serverAddress.sin_port = htons(serverPort);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server\n";
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to server at " << serverIP << ":" << serverPort << std::endl;

    while (true) {
        // Receive data from the server
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data from server\n";
        }
        else {
            char* token;
            bool isFirst = false;
            int rpc;
            token = strtok(buffer, " ");
            rpc = std::stoi(token);
            token = strtok(nullptr, " ");

            switch (rpc) {
            case RPC_ACCEPTED_CONNECTION:
            {
                std::cout << "Server accepted our connection." << std::endl;
                break;
            }
            case RPC_KICK_PLAYER:
            {
                std::cout << "The server closed our connection! (Kicked)" << std::endl;
                closesocket(clientSocket);
                break;
            }
            case RPC_SET_HEALTH:
            {
                if (token == nullptr || LOCAL_PLAYER.local_ped == nullptr || Player::isPaused() || (int)Player::getMenuID() != 41)
                    break;
                LOCAL_PLAYER.local_ped->setHealth(std::strtof(token, nullptr));
                break;
            }
            case RPC_SET_ARMOR:
            {
                if (token == nullptr || LOCAL_PLAYER.local_ped == nullptr || Player::isPaused() || (int)Player::getMenuID() != 41)
                    break;
                LOCAL_PLAYER.local_ped->setArmor(std::strtof(token, nullptr));
                break;
            }
            default:
            {
                std::cout << "Unknown RPC received! (code: " << rpc << ")" << std::endl;
                break;
            }
            }
        }
    }
    return 0ul;
}

/**
* @dev DLL's main function. Creates the mainThread which continues the code's execution.
**/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        #if DEBUG_MODE == true 
                AllocConsole();
                freopen("CONOUT$", "w", stdout);
                std::cout << "Console allocated.\n";
        #endif

        LOCAL_PLAYER = CLocal();
        LOCAL_PLAYER.local_ped = (Player*)(0xB6F5F0);
        
        // HOOKS ATTACH

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)oTakeDamageAddy, onPlayerTakeDamage); // Detour to your custom function
        DetourTransactionCommit();

        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainThread, hModule, 0, 0);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)tcpThread, NULL, 0, 0);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        // HOOKS DETACH

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)oTakeDamageAddy, onPlayerTakeDamage);
        DetourTransactionCommit();
    }
    return TRUE;
}

