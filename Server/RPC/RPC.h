/*************************************************************

    Every RPC is stored in this enum.
    RPC = Remote Procedure Call
    We use them to mark the meaning of the packages recv/send
    from/to the server.

 *************************************************************/

enum eServerRPC {
    RPC_EMPTY_DATA, // used for testing recv/send, make sure to send more than 0 bytes!
    RPC_ACCEPTED_CONNECTION,
    RPC_KICK_PLAYER,
    RPC_SET_HEALTH,
    RPC_SET_ARMOR,
    RPC_ON_PLAYER_TAKE_DAMAGE,
    RPC_ON_PLAYER_STATE_CHANGE,
    RPC_ON_PLAYER_DEATH,
    RPC_ON_PLAYER_SYNC_POSITION,
    RPC_SYNC_PLAYER_AT_POSITION
};