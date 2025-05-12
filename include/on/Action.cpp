#include "database/peer.hpp"
#include "network/packet.hpp"
#include "Action.hpp"

void Action(ENetEvent& event, const std::string_view text) 
{
    std::string_view to_slang = (text == "facepalm") ? "fp" : (text == "shrug") ? "idk" : (text == "foldarms") ? "fold" : (text == "fa") ? "fold" : (text == "stubborn") ? "fold" : text;
    std::string formatted_action{ "/" + std::string(to_slang) };
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (!_peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back())
            gt_packet(p, true, 0, {
                "OnAction", 
                formatted_action.c_str()
            });
    });
}
