#include "pch.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "on/EmoticonDataChanged.hpp"
#include "join_request.hpp"

#include "tools/string_view.hpp"

void join_request(ENetEvent event, const std::string& header) 
{
    try 
    {
        if (not create_rt(event, 2, 900)) throw std::runtime_error("");
        std::string big_name{readch(std::string{header}, '|')[3]};
        if (not alpha(big_name) or big_name.empty()) throw std::runtime_error("Sorry, spaces and special characters are not allowed in world or door names.  Try again.");
        std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); });
        auto w = std::make_unique<world>(world().read(big_name));
        if (w->name.empty()) 
        {
            const unsigned main_door = rand() % (100U * 60U / 100U - 4U) + 2U;
            
            std::vector<block> blocks(100 * 60, block{0, 0});
            std::ranges::transform(blocks, blocks.begin(), [&](auto& b) 
            {
                auto i = &b - &blocks[0];
                if (i >= 3700) 
                    b.bg = 14, // cave background
                    b.fg = (i >= 3800 and i < 5000 /* (above) lava level */ and rand() % 39 < 1) ? 10 : // rock
                        (i > 5000 and i < 5400 /* (above) bedrock level */ and rand() % 8 < 3) ? 4 : // lava
                        (i >= 5400) ? 8 : 2;
                if (i == 3600 + main_door) b.fg = 6; // main door
                if (i == 3700 + main_door) b.fg = 8; // bedrock (below main door)
                return b;
            });
            w->blocks = std::move(blocks);
            w->name = big_name; // init
        }
        {
            std::vector<std::byte> data(85 + w->name.length() + 5/*unknown*/ + (8 * w->blocks.size()) + 12/*initial drop*/, std::byte{ 00 });
            data[0] = std::byte{ 04 };
            data[4] = std::byte{ 04 };
            data[16] = std::byte{ 0x8 };
            unsigned char len = static_cast<unsigned char>(w->name.length());
            data[66] = static_cast<std::byte>(len);
            for (unsigned char i = 0; i < len; ++i)
                *reinterpret_cast<char*>(&data[68 + i]) = w->name[i];
            std::size_t y = w->blocks.size() / 100, x = w->blocks.size() / y;
            *reinterpret_cast<std::size_t*>(&data[68 + len]) = x;
            *reinterpret_cast<std::size_t*>(&data[72 + len]) = y;
            *reinterpret_cast<unsigned short*>(&data[76 + len]) = static_cast<unsigned short>(w->blocks.size());
            int pos = 85 + len;
            short i = 0;
            for (const auto& block : w->blocks)
            {
                auto [fg, bg, hits] = block;
                *reinterpret_cast<short*>(&data[pos]) = fg;
                *reinterpret_cast<short*>(&data[pos + 2]) = bg;
                if (fg == 6) // @todo all door labels & signs.
                {
                    _peer[event.peer]->pos.front() = (i % x) * 32;
                    _peer[event.peer]->pos.back() = (i / x) * 32;
                    _peer[event.peer]->rest_pos = _peer[event.peer]->pos; // @note static repsawn position
                    data.resize(data.size() + 8);
                    data[pos + 8] = std::byte{ 01 };
                    *reinterpret_cast<short*>(&data[pos + 9]) = 4;
                    for (int ii = 0; ii < 4; ++ii)
                        data[pos + 11 + ii] = static_cast<std::byte>("EXIT"[ii]);
                    pos += 8;
                }
                else if (fg == 242) // @todo all locks
                {
                    data.resize(data.size() + 15);
                    data[pos + 8] = std::byte{ 03 };
                    data[pos + 9] = std::byte{ 01 };
                    *reinterpret_cast<int*>(&data[pos + 10]) = 1; // @note owner user ID
                    data[pos + 14] = std::byte{ 01 }; // @note number of admins
                    *reinterpret_cast<int*>(&data[pos + 18]) = -100; // @note default world bpm
                    *reinterpret_cast<int*>(&data[pos + 22]) = 1; // @note list of admins
                    pos += 14;
                }
                pos += 8;
                ++i;
            }
            enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
        } // @note delete data

        for (const auto& [uid, id, count, position] : w->ifloats)
        {
            std::vector<std::byte> compress = compress_state({
                .type = 14, 
                .netid = -1, 
                .id = id, 
                .pos = {position[0] * 32, position[1] * 32}
            });
            *reinterpret_cast<int*>(&compress[8]) = uid;
            *reinterpret_cast<float*>(&compress[16]) = static_cast<float>(count);
            send_data(*event.peer, compress);
        }
        if (std::find(_peer[event.peer]->recent_worlds.begin(), _peer[event.peer]->recent_worlds.end(), w->name) == _peer[event.peer]->recent_worlds.end()) 
        {
            std::ranges::rotate(_peer[event.peer]->recent_worlds, _peer[event.peer]->recent_worlds.begin() + 1);
            _peer[event.peer]->recent_worlds.back() = w->name;
        }
        _peer[event.peer]->lobby = false;
        EmoticonDataChanged(event);
        _peer[event.peer]->netid = ++w->visitors;
        gt_packet(*event.peer, false, {
            "OnSpawn", 
            std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|`w{}``\ncountry|{}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local\n",
            _peer[event.peer]->netid, _peer[event.peer]->user_id, static_cast<int>(_peer[event.peer]->pos[0]), static_cast<int>(_peer[event.peer]->pos[1]), _peer[event.peer]->ltoken[0], "jp").c_str()
        });
        peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
        {
            if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back() and _peer[&p]->user_id not_eq _peer[event.peer]->user_id)
            {
                gt_packet(p, false, {
                    "OnSpawn", 
                    std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|{}\ncountry|{}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\n",
                    _peer[&p]->netid, _peer[&p]->user_id, static_cast<int>(_peer[&p]->pos.front()), static_cast<int>(_peer[&p]->pos.back()), _peer[&p]->ltoken[0], "jp").c_str()
                });
                gt_packet(p, false, {
                    "OnConsoleMessage", 
                    std::format("`5<`w{}`` entered, `w{}`` others here>``", _peer[event.peer]->ltoken[0], w->visitors).c_str()
                });
                gt_packet(p, false, {
                    "OnTalkBubble", 
                    _peer[event.peer]->netid, std::format("`5<`w{}`` entered, `w{}`` others here>``", _peer[event.peer]->ltoken[0], w->visitors).c_str()});
            }
        });
        gt_packet(*event.peer, false, {
            "OnConsoleMessage", 
            std::format("World `w{}`` entered.  There are `w{}`` other people here, `w{}`` online.", w->name, w->visitors - 1, peers().size()).c_str()});
        inventory_visuals(event);
        worlds.emplace(w->name, *w);
    }
    catch (const std::exception& exc)
    {
        printf("catch error during join request: %s\n", exc.what());
        if (not std::string{exc.what()}.empty()) gt_packet(*event.peer, false, { "OnConsoleMessage", exc.what() });
        gt_packet(*event.peer, false, { "OnFailedToEnterWorld" });
    }
}