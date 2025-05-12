#include "pch.hpp"
#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "on/EmoticonDataChanged.hpp"
#include "join_request.hpp"

#include "tools/string_view.hpp"
#include "tools/randomizer.hpp"

constexpr std::array<std::byte, 4> EXIT{
    std::byte{ 0x45 },
    std::byte{ 0x58 },
    std::byte{ 0x49 },
    std::byte{ 0x54 }
};

void join_request(ENetEvent event, const std::string& header, const std::string_view world_name = "") 
{
    try 
    {
        if (not create_rt(event, 2, 900)) throw std::runtime_error("");
        std::string big_name{world_name.empty() ? readch(std::string{header}, '|')[3] : world_name};
        if (not alpha(big_name) || big_name.empty()) throw std::runtime_error("Sorry, spaces and special characters are not allowed in world or door names.  Try again.");
        std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); }); // @note start -> START
        std::unique_ptr<world> w = std::make_unique<world>(std::move(world().read(big_name)));
        if (w->name.empty()) 
        {
            const unsigned main_door = randomizer(2, 100 * 60 / 100 - 4);
            
            std::vector<block> blocks(100 * 60, block{0, 0});
            std::ranges::transform(blocks, blocks.begin(), [&](block& b) 
            {
                long long i = &b - &blocks[0];
                if (i >= 3700) 
                    b.bg = 14, // cave background
                    b.fg = (i >= 3800 && i < 5000 /* (above) lava level */ && !randomizer(0, 38)) ? 10 : // rock
                        (i > 5000 && i < 5400 /* (above) bedrock level */ && randomizer(0, 8) < 3) ? 4 : // lava
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
            for (const auto& [fg, bg, label, hits] : w->blocks)
            {
                *reinterpret_cast<short*>(&data[pos]) = fg; pos += sizeof(short);
                *reinterpret_cast<short*>(&data[pos]) = bg; pos += sizeof(short);
                pos += sizeof(short); // @todo
                pos += sizeof(short); // @todo (water = 00 04)
                switch (items[fg].type)
                {
                    case std::byte{ type::FOREGROUND }: 
                    case std::byte{ type::BACKGROUND }:
                        break;

                    case std::byte{ type::LOCK }: 
                    {
                        data[pos - 2] = std::byte{ 01 };
                        size_t admins = std::ranges::count_if(w->admin, std::identity{});
                        data.resize(data.size() + 14 + ((admins) * 4));
                        data[pos] = std::byte{ 03 }; pos += sizeof(std::byte);
                        data[pos] = std::byte{ 00 }; pos += sizeof(std::byte);
                        *reinterpret_cast<int*>(&data[pos]) = w->owner; pos += sizeof(int);
                        *reinterpret_cast<int*>(&data[pos]) = admins + 1; pos += sizeof(int);
                        *reinterpret_cast<int*>(&data[pos]) = -100; pos += sizeof(int);
                        /* @todo admin list */

                        break;
                    }
                    case std::byte{ type::MAIN_DOOR }: 
                    {
                        data[pos - 2] = std::byte{ 01 };
                        _peer[event.peer]->pos.front() = (i % x) * 32;
                        _peer[event.peer]->pos.back() = (i / x) * 32;
                        _peer[event.peer]->rest_pos = _peer[event.peer]->pos; // @note static repsawn position
                        data.resize(data.size() + 8);
                        data[pos] = std::byte{ 01 }; pos += sizeof(std::byte);
                        *reinterpret_cast<short*>(&data[pos]) = 4; pos += sizeof(short); // @note length of "EXIT"
                        *reinterpret_cast<std::array<std::byte, 4>*>(&data[pos]) = EXIT; pos += sizeof(std::array<std::byte, 4>);
                        data[pos] = std::byte{ 00 }; pos += sizeof(std::byte); // @note '\0'
                        
                        break;
                    }
                    case std::byte{ type::DOOR }:
                    {
                        data[pos - 2] = std::byte{ 01 };
                        short len{ static_cast<short>(label.length()) };
                        data.resize(data.size() + 4 + len);
                        data[pos] = std::byte{ 01 }; pos += sizeof(std::byte);
                        *reinterpret_cast<short*>(&data[pos]) = len; pos += sizeof(short);
                        if (not label.empty())
                            for (short ii = 0; ii < len; ++ii)
                                data[pos] = static_cast<std::byte>(label[ii]), pos += sizeof(std::byte);
                        data[pos] = std::byte{ 00 }; pos += sizeof(std::byte); // @note '\0'
                        break;
                    }
                    case std::byte{ type::SIGN }:
                    {
                        data[pos - 2] = std::byte{ 0x19 };
                        short len{ static_cast<short>(label.length()) };
                        data.resize(data.size() + 7 + len);
                        data[pos] = std::byte{ 02 }; pos += sizeof(std::byte);
                        *reinterpret_cast<short*>(&data[pos]) = len; pos += sizeof(short);
                        if (not label.empty())
                            for (short ii = 0; ii < len; ++ii)
                                data[pos] = static_cast<std::byte>(label[ii]), pos += sizeof(std::byte);
                        *reinterpret_cast<int*>(&data[pos]) = -1; pos += sizeof(int);
                        break;
                    }
                    default:
                        data.resize(data.size() + 16); // @todo
                        break;
                }
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
        } // @note delete compress
        if (std::find(_peer[event.peer]->recent_worlds.begin(), _peer[event.peer]->recent_worlds.end(), w->name) == _peer[event.peer]->recent_worlds.end()) 
        {
            std::ranges::rotate(_peer[event.peer]->recent_worlds, _peer[event.peer]->recent_worlds.begin() + 1);
            _peer[event.peer]->recent_worlds.back() = w->name;
        }
        _peer[event.peer]->post_enter.unlock();
        EmoticonDataChanged(event);
        _peer[event.peer]->netid = ++w->visitors;

        gt_packet(*event.peer, false, 0, {
            "OnSpawn", 
            std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|`w{}``\ncountry|us\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local\n",
            _peer[event.peer]->netid, _peer[event.peer]->user_id, static_cast<int>(_peer[event.peer]->pos.front()), static_cast<int>(_peer[event.peer]->pos.back()), _peer[event.peer]->ltoken[0]).c_str()
        });
        peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
        {
            if (!_peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()  && 
                /* @todo */_peer[&p]->user_id != _peer[event.peer]->user_id)
            {
                gt_packet(p, false, 0, {
                    "OnSpawn", 
                    std::format("spawn|avatar\nnetID|{}\nuserID|{}\ncolrect|0|0|20|30\nposXY|{}|{}\nname|`w{}``\ncountry|us\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\n",
                    _peer[&p]->netid, _peer[&p]->user_id, static_cast<int>(_peer[&p]->pos.front()), static_cast<int>(_peer[&p]->pos.back()), _peer[&p]->ltoken[0]).c_str()
                });
                std::string enter_message{ std::format("`5<`w{}`` entered, `w{}`` others here>``", _peer[event.peer]->ltoken[0], w->visitors) };
                gt_packet(p, false, 0, {
                    "OnConsoleMessage", 
                    enter_message.c_str()
                });
                gt_packet(p, false, 0, {
                    "OnTalkBubble", 
                    1u, 
                    enter_message.c_str()
                });
            } // @note delete enter_message
        });
        gt_packet(*event.peer, false, 0, {
            "OnConsoleMessage", 
            std::format("World `w{}`` entered.  There are `w{}`` other people here, `w{}`` online.", w->name, w->visitors - 1, peers().size()).c_str()});
        inventory_visuals(event);
        worlds.emplace(w->name, *w);
    }
    catch (const std::exception& exc)
    {
        if (exc.what() && *exc.what()) gt_packet(*event.peer, false, 0, { "OnConsoleMessage", exc.what() });
        gt_packet(*event.peer, false, 0, { "OnFailedToEnterWorld" });
    }
}