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
#include "../Player/CLocal.h"

#include "../../Server/RPC/RPC.h"

#include "../Hooks/hooks.hpp"

#define DEBUG_MODE true
#define MAX_PLAYERS 10

SOCKET clientSocket;
CLocal LOCAL_PLAYER = CLocal();

CLocal* PLAYERS = new CLocal[MAX_PLAYERS];

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
            LOCAL_PLAYER.ped->sendTextBox(LOCAL_PLAYER.ped->getLastTypedCharacters());
            LOCAL_PLAYER.ped->resetLastTypedCharacters();
        }
        else if (GetAsyncKeyState(VK_F6) & 1) {
            LOCAL_PLAYER.ped->resetLastTypedCharacters();
        }
        else if (GetAsyncKeyState(VK_F7) & 1) {
            LOCAL_PLAYER.ped->logPosition();
            LOCAL_PLAYER.ped->setPosition(CVector3D(LOCAL_PLAYER.ped->getPosition().x, LOCAL_PLAYER.ped->getPosition().y, LOCAL_PLAYER.ped->getPosition().z + 2));
        }

        // EVENTS

        // OnPlayerTakeDamage
        if (e_onTakeDamage) 
        {
            e_onTakeDamage = false;
            char data[20];
            std::sprintf(data, "%d %f", RPC_ON_PLAYER_TAKE_DAMAGE, LOCAL_PLAYER.ped->getHealth());
            int bytesSent = send(clientSocket, data, sizeof(data), 0);
        }

        // OnPlayerStateChange
        if (LOCAL_PLAYER.ped != nullptr && !Player::isPaused() && (int)Player::getMenuID() == 41) {
            LOCAL_PLAYER.newState = LOCAL_PLAYER.ped->getState();
            if (LOCAL_PLAYER.newState != LOCAL_PLAYER.oldState) {
                if (LOCAL_PLAYER.oldState != -1) {
                    char data[20];
                    std::sprintf(data, "%d %d %d", RPC_ON_PLAYER_STATE_CHANGE, LOCAL_PLAYER.newState, LOCAL_PLAYER.oldState);
                    int bytesSent = send(clientSocket, data, sizeof(data), 0);
                }
                LOCAL_PLAYER.oldState = LOCAL_PLAYER.newState;
            }
        }

        // OnPlayerSyncPosition
        if (LOCAL_PLAYER.ped != nullptr && !Player::isPaused() && *(void**)0xC1703C /* RENDERWARE INIT */ && LOCAL_PLAYER.ped->getMenuID() == 41)
        {
            char data[40];
            std::sprintf(data, "%d %f %f %f", RPC_ON_PLAYER_SYNC_POSITION, 
                LOCAL_PLAYER.ped->getPosition().x, LOCAL_PLAYER.ped->getPosition().y, LOCAL_PLAYER.ped->getPosition().z);
            int bytesSent = send(clientSocket, data, sizeof(data), 0);
        }

        Sleep(500);
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
    const char* serverIP = "188.25.182.38";
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
                LOCAL_PLAYER.sID = std::atoi(token);
                std::cout << "Server accepted our connection. (ID #" << LOCAL_PLAYER.sID << ")" << std::endl;
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
                if (token == nullptr || LOCAL_PLAYER.ped == nullptr || Player::isPaused() || (int)Player::getMenuID() != 41)
                    break;
                LOCAL_PLAYER.ped->setHealth(std::strtof(token, nullptr));
                if (LOCAL_PLAYER.ped->getHealth() <= 0.0f) {
                    char data[20];
                    std::sprintf(data, "%d", RPC_ON_PLAYER_DEATH);
                    int bytesSent = send(clientSocket, data, sizeof(data), 0);
                }
                break;
            }
            case RPC_SET_ARMOR:
            {
                if (token == nullptr || LOCAL_PLAYER.ped == nullptr || Player::isPaused() || (int)Player::getMenuID() != 41)
                    break;
                LOCAL_PLAYER.ped->setArmor(std::strtof(token, nullptr));
                break;
            }
            case RPC_SYNC_PLAYER_AT_POSITION:
            {
                if (token == nullptr || LOCAL_PLAYER.ped == nullptr || Player::isPaused() || (int)Player::getMenuID() != 41)
                    break;
                
                int sID = std::atoi(token);
                token = strtok(nullptr, " ");

                float x = std::atof(token);
                token = strtok(nullptr, " ");

                float y = std::atof(token);
                token = strtok(nullptr, " ");

                float z = std::atof(token);
                token = strtok(nullptr, " ");

                CVector3D player_position = CVector3D(x, y, z);
                CVector3D local_position = CVector3D(LOCAL_PLAYER.ped->getPosition().x, LOCAL_PLAYER.ped->getPosition().y, LOCAL_PLAYER.ped->getPosition().z);

                if (PLAYERS[sID].ped == nullptr) {
                    //if (CVector3D::isInCircularRange(player_position, local_position, 30.0f)) {
                        CVector3D position = CVector3D(x, y, z);
                        PLAYERS[sID].ped = oAddPedAddy(PED_TYPE_CIVMALE, MODEL_PLAYER, position, 0);
                        PLAYERS[sID].sID = sID;
                    //}
                }
                else {
                    //if (CVector3D::isInCircularRange(player_position, local_position, 30.0f)) {
                    oRemovePedAddy(PLAYERS[sID].ped);
                    PLAYERS[sID].ped = oAddPedAddy(PED_TYPE_CIVMALE, MODEL_PLAYER, player_position, 0);
                    //}
                    //else {
                        //oRemovePedAddy(PLAYERS[sID].ped);
                        //PLAYERS[sID].ped = nullptr;
                    //}
                }
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
        LOCAL_PLAYER.ped = (Player*)(0xB6F5F0);
        
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

