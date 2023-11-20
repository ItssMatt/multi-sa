# About
 A very basic C++ extension that transforms the GTA: San Andreas game from singleplayer to multiplayer.

# What does it have?
The project offers the server, the .dll, and the injector.
- The server is handling all the connections, a few events ( OnPlayerTakeDamage, OnPlayerSyncPosition, OnPlayerStateChange, OnPlayerDeath) and sync for players' health and position.
- The dll is setting up the game, connects to the server (hardcoded IP and port) and ties it to the player's game by hooking to certain functions and modifying the game's memory, and handles the sync data received by the server.
- The injector takes the Client.dll, opens "gta_sa.exe" process, allocates memory for the dll and injects it into the game.

# What is missing?
The project reached the initial target when I started it. But there are still many things that are missing:

- Many events are not implemented (most important are OnPlayerStreamIn, OnPlayerStreamOut, OnPlayerGiveDamage ...)
- A Streamer for players is a must! (load only the players that are in the range of local_player)
- Many crashes need to be fixed.
- All vehicles must be removed and handled by the server only!
- Animations must be added, as well as facing angle + sync for both of them.
- DirectX9 should be injected to create a chat and other things on screen.
