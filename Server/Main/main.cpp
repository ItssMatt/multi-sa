/*************************************************************

	This file is the main entry point for the .exe that will
	be used to host and run a server locally. It must send
	and receive data from all the clients and make sure all
	players are synced.

 *************************************************************/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstring>
#include <fstream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "../nlohmann/json.hpp"

#include "../CPlayer/CPlayer.h"
#include "../RPC/RPC.h"

 /**
 * @dev This function is used to send RPCs to players. This is how we send information to them!
 **/
int sendRPCToClient(SOCKET client, eServerRPC RPC, int optional = 0) {
    char data[20];
    std::sprintf(data, "%d", RPC);
    switch (RPC) {
    case RPC_ACCEPTED_CONNECTION:
    {
        return send(client, (char*)data, sizeof(data), 0);
        break;
    }
    case RPC_KICK_PLAYER:
    {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (CPlayer::gPlayers[i].player_socket != client)
                continue;
            std::cout << "Player #" << CPlayer::gPlayers[i].id << " successfully kicked from the server." << std::endl;
            CPlayer::gPlayers[i].validPlayer = false;
            int bytes = send(client, (char*)data, sizeof(data), 0);
            Sleep(100);
            closesocket(CPlayer::gPlayers[i].player_socket);
            return bytes;
        }
        std::cout << "The specified player does not exist." << std::endl;
        break;
    }
    }
    // should be unreachable
    return -1;
}

/**
* @dev Server's sync thread. Here we sync every connected player. We can also use this function for testing.
**/
void syncThread(void)
{
    while (true) {
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            std::cout << "Sent\n";
            sendRPCToClient(CPlayer::gPlayers[0].player_socket, RPC_KICK_PLAYER);
        }
        Sleep(10);
    }
}

/**
* @dev Server's main function. Here we read the config file, setup the server, run it and process/recv players/data.
**/
int main() {

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)syncThread, NULL, 0, NULL);

    std::ifstream configFile("config.json");

    // Check if the file is open
    if (!configFile.is_open()) {
        std::cerr << "Error opening config.json\n";
        return -1;
    }

    nlohmann::json config;
    try {
        configFile >> config;
    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        configFile.close();
        return -1;
    }

    int port = config["port"];

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return -1;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket\n";
        WSACleanup();
        return -1;
    }

    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket\n";
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket\n";
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Multi-SA started!\n";
    std::cout << "Server listening on port " << port << ".\n";

    while (true) {
        // Accept incoming connections
        sockaddr_in clientAddress;
        int clientAddrSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection\n";
            continue; // Continue to accept next connection
        }

        int id = CPlayer::getAvailablePlayerID();
        CPlayer::gPlayers[id].id = id;
        CPlayer::gPlayers[id].player_socket = clientSocket;
        CPlayer::gPlayers[id].validPlayer = true;
        std::cout << "Connection accepted from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " (pID: " << id << ") " << "[" << CPlayer::getConnectedPlayers() << "/" << MAX_PLAYERS << "]" << "\n";


        // Here we should sanitize the connection before accepting it.

        sendRPCToClient(clientSocket, RPC_ACCEPTED_CONNECTION);


        // Handle client's request, send/receive data, etc.
        char buffer[1024];
        int bytesReceived;

        // Receive data from the client
        while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
   
        }

        if (bytesReceived <= 0) {
            // Client disconnected
            USHORT id;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;
                id = CPlayer::gPlayers[i].id;
                CPlayer::gPlayers[i].validPlayer = false;
            }

            std::cout << "Client " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " (pID: " << id << ") " << "has disconnected from the server. " << "[" << CPlayer::getConnectedPlayers() << "/" << MAX_PLAYERS << "]" << std::endl;
        }
    }
    return 0;
}