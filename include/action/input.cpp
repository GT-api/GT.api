#include "pch.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "input.hpp"

#include "tools/string_view.hpp"
#include "commands/commands"

#include <future>

void input(ENetEvent event, const std::string& header)
{
    if (not create_rt(event, 1, 400)) return;

    std::string text{readch(std::string{header}, '|')[4]};
    {
        if (text.empty()) return;

        auto start = std::ranges::find_if(text, [](unsigned char c) { return !std::isspace(c); });
        auto end = std::ranges::find_if(text.rbegin(), text.rend(), [](unsigned char c) { return !std::isspace(c); }).base();
        if (start < end) text.assign(start, end);
    } // @note delete start, end | 解放する: start, end
    _peer[event.peer]->messages.push_back(std::chrono::steady_clock::now());
    if (_peer[event.peer]->messages.size() > 5) _peer[event.peer]->messages.pop_front();
    if (_peer[event.peer]->messages.size() == 5 and std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _peer[event.peer]->messages.front()).count() < 6)
        gt_packet(*event.peer, false, {
            "OnConsoleMessage", 
            "`6>>`4Spam detected! ``Please wait a bit before typing anything else.  "  
            "Please note, any form of bot/macro/auto-paste will get all your accounts banned, so don't do it!"
        });
    else if (text.starts_with('/')) 
    {
        action(*event.peer, "log", std::format("msg| `6{}``", text).c_str());

        std::string_view text_view = text.substr(1, text.find(' ') - 1); // @note e.g. /warp {} -> warp. this excludes the arguement and slash.
        if (auto it = cmd_pool.find(text_view); it not_eq cmd_pool.end()) 
            std::async(std::launch::async, it->second, std::ref(event), std::move(text.substr(1)));
        else 
            action(*event.peer, "log", "msg|`4Unknown command.`` Enter `$/?`` for a list of valid commands.");
    }
    else if (text.back() == ' ' and text.length() > 1) text.pop_back(); // @note trim back spaces. "test "
    else if (text.front() == ' ' and text.length() > 1) text = text.substr(1, text.length() - 1); // @note trim front spacing. " test"
    else if (text.empty()) return; // @note ignore empty messages.
    else peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back())
            gt_packet(p, false, {
                "OnTalkBubble", 
                1u, 
                std::format("CP:0_PL:0_OID:_player_chat={}", text).c_str()
            }),
            gt_packet(p, false, {
                "OnConsoleMessage", 
                std::format("CP:0_PL:0_OID:_CT:[W]_ `6<`w{}``>`` `$`${}````", _peer[event.peer]->ltoken[0], text).c_str()
            });
    });
}
