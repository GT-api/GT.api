#include "pch.hpp"
#include "database/peer.hpp"
#include "network/packet.hpp"
#include "action/join_request.hpp"
#include "action/quit_to_exit.hpp"
#include "warp.hpp"

void warp(ENetEvent& event, const std::string_view text)
{
    std::string world_name{ text.substr(sizeof("warp ")-1) };
    std::ranges::transform(world_name, world_name.begin(), [](char c) { return std::toupper(c); });

    action(*event.peer, "log", std::format("msg| `6/warp {}``", world_name));
    gt_packet(*event.peer, false, 0, { "OnSetFreezeState", 1 });
    action(*event.peer, "log", std::format("msg|Magically warping to world `5{}``...", world_name));
    quit_to_exit(event, "", true);
    join_request(event, "", world_name);
}
