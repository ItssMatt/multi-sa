/*************************************************************

    Every RPC is stored in this enum.

 *************************************************************/

enum eServerRPC {
    RPC_EMPTY_DATA, // used for testing recv/send, make sure to send more than 0 bytes!
    RPC_ACCEPTED_CONNECTION,
    RPC_KICK_PLAYER
};