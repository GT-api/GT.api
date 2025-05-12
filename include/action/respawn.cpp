#include "pch.hpp"
#include "database/peer.hpp"
#include "network/packet.hpp"
#include "respawn.hpp"

using namespace std::chrono;

void respawn(ENetEvent event, const std::string& header) 
{
    gt_packet(*event.peer, true, 0, { 
        "OnSetFreezeState", 
        2 
    });
    gt_packet(*event.peer, true, 0,{ "OnKilled" });
    // @note wait 1900 milliseconds...
    gt_packet(*event.peer, true, 1900, {
        "OnSetPos", 
        std::vector<float>{_peer[event.peer]->rest_pos.front(), _peer[event.peer]->rest_pos.back()}
    });
    gt_packet(*event.peer, true, 1900, { "OnSetFreezeState" });
}