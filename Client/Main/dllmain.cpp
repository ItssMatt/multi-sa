/*************************************************************

	This file is the main entry point for the .dll that will
	be injected into the gta_sa.exe file. It must hijack the
	objects within the game's source code and manipulate the
	memory according to the server's sync.

 *************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "../Player/Player.h"

#include "../../Server/RPC/RPC.h"

#define DEBUG_MODE true

SOCKET clientSocket;

/**
* @dev The main thread's function. This is the core of the client, here we will handle the data received/sent from/to the server. 
**/
ULONG mainThread(void* hModule) 
{
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
            int rpc = std::stoi(buffer);

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

        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)mainThread, hModule, 0, 0);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)tcpThread, NULL, 0, 0);
    }
    return TRUE;
}

