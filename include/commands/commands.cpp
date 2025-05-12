#include "pch.hpp"
#include "database/peer.hpp"
#include "network/packet.hpp"
#include "on/Action.hpp"
#include "find.hpp"
#include "warp.hpp"
#include "commands.hpp"

std::unordered_map<std::string_view, std::function<void(ENetEvent&, const std::string_view)>> cmd_pool
{
    {"help", [](ENetEvent& event, const std::string_view text) 
    {
        action(*event.peer, "log", "msg|>> Commands: /find /warp {world name} /wave /dance /love /sleep /facepalm /fp /smh /yes /no /omg /idk /shrug /furious /rolleyes /foldarms /stubborn /fold /dab /sassy /dance2 /march /grumpy /shy");
    }},
    {"find", &find},
    {"warp", &warp},
    {"wave", &Action}, {"dance", &Action}, {"love", &Action}, {"sleep", &Action}, {"facepalm", &Action}, {"fp", &Action}, {"smh", &Action}, {"yes", &Action}, 
    {"no", &Action}, {"omg", &Action}, {"idk", &Action}, {"shrug", &Action}, {"furious", &Action}, {"rolleyes", &Action}, {"foldarms", &Action}, {"fa", &Action}, 
    {"stubborn", &Action}, {"fold", &Action}, {"dab", &Action}, {"sassy", &Action}, {"dance2", &Action}, {"march", &Action}, {"grumpy", &Action}, {"shy", &Action}
};