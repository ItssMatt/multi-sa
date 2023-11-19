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
#include <vector>
#include <thread>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#include "../nlohmann/json.hpp"

#include "../../Client/Player/PedStates.h"
#include "../CPlayer/CPlayer.h" // server-side Player
#include "../RPC/RPC.h"

 /**
 * @dev This function is used to send RPCs to players. This is how we send information to them!
 **/
template<class c>
int sendRPCToClient(SOCKET client, eServerRPC RPC, c parameter = 0) {
    char data[20];
    std::sprintf(data, "%d", RPC);
    switch (RPC) {
    case RPC_ACCEPTED_CONNECTION:
    {
        char buffer[20];
        std::sprintf(buffer, "%d %d", RPC, parameter);
        return send(client, (char*)buffer, sizeof(buffer), 0);
        break;
    }
    case RPC_KICK_PLAYER:
    {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (CPlayer::gPlayers[i].player_socket != client || !CPlayer::gPlayers[i].validPlayer)
                continue;
            std::cout << "Player #" << CPlayer::gPlayers[i].id << " successfully kicked from the server." << std::endl;
            CPlayer::gPlayers[i].validPlayer = false;
            int bytes = send(client, (char*)data, sizeof(data), 0);
            closesocket(CPlayer::gPlayers[i].player_socket);
            return bytes;
        }
        std::cout << "The specified player does not exist." << std::endl;
        break;
    }
    case RPC_SET_HEALTH:
    {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (CPlayer::gPlayers[i].player_socket != client)
                continue;
            //std::cout << "SetPlayerHealth(" << CPlayer::gPlayers[i].id << "," << parameter << ")" << std::endl;
            CPlayer::gPlayers[i].health = parameter;
            char buffer[50];
            std::sprintf(buffer, "%s %.2f", data, parameter);
            return send(client, buffer, sizeof(buffer), 0);
            break;
        }
        break;
    }
    case RPC_SET_ARMOR:
    {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (CPlayer::gPlayers[i].player_socket != client)
                continue;
            //std::cout << "SetPlayerArmor(" << CPlayer::gPlayers[i].id << "," << parameter << ")" << std::endl;
            CPlayer::gPlayers[i].armor = parameter;
            char buffer[50];
            std::sprintf(buffer, "%s %.2f", data, parameter);
            return send(client, buffer, sizeof(buffer), 0);
            break;
        }
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
        // TESTING
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            std::cout << "Sent\n";
            sendRPCToClient<float>(CPlayer::gPlayers[0].player_socket, RPC_SET_HEALTH, CPlayer::gPlayers[0].health - 10);
        }
        else if (GetAsyncKeyState(VK_F2) & 1) {
            std::cout << "Sent\n";
            sendRPCToClient<float>(CPlayer::gPlayers[0].player_socket, RPC_SET_ARMOR, CPlayer::gPlayers[0].armor + 5);
        }
        // SYNC
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!CPlayer::gPlayers[i].validPlayer)
                continue;
            //sendRPCToClient(CPlayer::gPlayers[i].player_socket, RPC_SET_HEALTH, CPlayer::gPlayers[i].health); // Health Sync
            //sendRPCToClient(CPlayer::gPlayers[i].player_socket, RPC_SET_ARMOR, CPlayer::gPlayers[i].armor); // Armor Sync
        }
        Sleep(10);
    }
}

/**
* @dev Client Handler. This function runs on a separate thread for every player to recv from it.
**/
void clientHandler(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived;

    while (true) {
        bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            // Client disconnected
            USHORT id;
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;
                id = CPlayer::gPlayers[i].id;
                CPlayer::gPlayers[i].validPlayer = false;
            }

            std::cout << "Client " << " (pID: #" << id << ") " << "has disconnected from the server. " << "[" << CPlayer::getConnectedPlayers() << "/" << MAX_PLAYERS << "]" << std::endl;
            break;
        }

        //std::cout << buffer << std::endl;
        char* token;
        bool isFirst = false;
        int rpc;
        token = strtok(buffer, " ");
        rpc = std::stoi(token);
        token = strtok(nullptr, " ");

        switch (rpc)
        {
        case RPC_ON_PLAYER_TAKE_DAMAGE:
        {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;

                float newHealth = std::strtof(token, nullptr);
                CPlayer::gPlayers[i].health = std::max<float>(0.0f, CPlayer::gPlayers[i].health - (CPlayer::gPlayers[i].health - newHealth)); // if the player cheated his health, kill
                sendRPCToClient<float>(CPlayer::gPlayers[i].player_socket, RPC_SET_HEALTH, CPlayer::gPlayers[i].health);
                std::cout << "OnPlayerTakeDamage(playerid=" << i << ", newHealth=" << newHealth << ")" << std::endl;
                break;
            }
            break;
        }
        case RPC_ON_PLAYER_STATE_CHANGE:
        {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;

                int newState = std::atoi(token);
                token = strtok(nullptr, " ");
                int oldState = std::atoi(token);

                std::cout << "OnPlayerStateChange(playerid=" << i << ", newState=" << stateToChar(newState) << ", oldState=" << stateToChar(oldState) << ")" << std::endl;
                break;
            }
            break;
        }
        case RPC_ON_PLAYER_DEATH:
        {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;

                std::cout << "OnPlayerDeath(playerid=" << i << ")" << std::endl;
                break;
            }
            break;
        }
        case RPC_ON_PLAYER_SYNC_POSITION:
        {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (CPlayer::gPlayers[i].player_socket != clientSocket)
                    continue;

                float x = std::atof(token);
                token = strtok(nullptr, " ");

                float y = std::atof(token);
                token = strtok(nullptr, " ");

                float z = std::atof(token);
                token = strtok(nullptr, " ");

                CPlayer::gPlayers[i].position = CVector3D(x, y, z);

                for (int ii = 0; ii < MAX_PLAYERS; ii++) {
                    if (i == ii) continue;
                    char buffer[50];
                    std::sprintf(buffer, "%d %d %f %f %f", RPC_SYNC_PLAYER_AT_POSITION, i, x, y, z);
                    send(CPlayer::gPlayers[ii].player_socket, buffer, sizeof(buffer), 0);
                }

                std::cout << "OnPlayerSyncPosition(playerid=" << i << ", x=" << x << ", y=" << y << ", z=" << z << ")" << std::endl;
                break;
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

    closesocket(clientSocket);
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

    // Every client works on a separate thread with the server
    std::vector<std::thread> clientThreads;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        sockaddr_in clientAddress;
        int clientAddrSize = sizeof(clientAddress);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection\n";
            continue;
        }

        int id = CPlayer::getAvailablePlayerID();
        CPlayer::gPlayers[id].id = id;
        CPlayer::gPlayers[id].player_socket = clientSocket;
        CPlayer::gPlayers[id].validPlayer = true;

        CPlayer::gPlayers[id].health = 100;
        CPlayer::gPlayers[id].armor = 0;
        CPlayer::gPlayers[id].position = CVector3D();

        std::cout << "Connection accepted from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << " (pID: " << id << ") " << "[" << CPlayer::getConnectedPlayers() << "/" << MAX_PLAYERS << "]" << "\n";
    
        // Here we should sanitize the connection before accepting it.

        sendRPCToClient<int>(clientSocket, RPC_ACCEPTED_CONNECTION, id);

        clientThreads.emplace_back(clientHandler, clientSocket);
    }

    for (auto& thread : clientThreads) {
        thread.join();
    }
    return 0;
}