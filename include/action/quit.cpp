#include "database/peer.hpp"
#include "quit.hpp"

void quit(ENetEvent event, const std::string& header) 
{
    if (event.peer == nullptr) return;
    if (event.peer->data not_eq nullptr) 
    {
        event.peer->data = nullptr;
        _peer.erase(event.peer);
    }
    enet_peer_reset(event.peer);
}