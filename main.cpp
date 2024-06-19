#include <iostream>
#include <fstream>
#include <format> /* enhanced string arguements & better logging */
#include <thread> /* std::jthread() & std::chrono / std::this_thread */
#include <vector>
#include <numeric> /* accumulate for items.dat hash (acc for short) */
#include <unordered_map>
#include <ranges>
#include <algorithm>
#include <random> /* for random generator */

#include "include/database/items.hpp"
#include "include/database/sqlite3.hpp"
#define ENET_IMPLEMENTATION
#include "include/network/enet.hpp"
#include "include/database/peer.hpp"
#include "include/network/packet.hpp"
#include "include/database/world.hpp" /* using a packet creation. ugh- I will improve later... */

using namespace std::literals;

/* 
@param str the whole content
@brief reads a string and seperates the pipes '|' and stores those pieces of info into a vector.
@return std::vector<std::string> of all the seperations of the pipes '|' e.g. Name|myName -> readpipe[1] = myName
*/
std::vector<std::string> readpipe(const std::string& str) {
    std::vector<std::string> separations;
    for (auto&& part : str | std::views::split('|'))
        separations.emplace_back(std::string(std::ranges::begin(part), std::ranges::end(part)));
    return separations;
}

int rand(const int min, const int max) {
    static std::random_device device;
    static std::mt19937 mt(device());
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(mt);
}

/** @brief An adaptive order-2 PPM range coder */ // -> compress.o
int enet_host_compress_with_range_coder (ENetHost* host);

namespace github {
    /* @brief checks if commit is the latest on the main branch. */
    void sync(const char* commit); 
}

int main() {
    github::sync("7ae48ab72e4d8e0dfb4c979a190f20dba215291c");
    if (enet_initialize() not_eq 0) 
        std::cerr << "enet_initialize() failed" << std::endl, std::cin.ignore();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};

    server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
            if (not file) std::cout << "failed to open items.dat" << std::endl, std::cin.ignore();
            if (file.tellg() < 0) std::cout << "items.dat contains no data" << std::endl, std::cin.ignore();
        std::streamsize im_size = file.tellg();
        im_data.resize(im_size + 60);
        for (int i = 0; i < 5; ++i)
            *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) =  std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
        *reinterpret_cast<int*>(im_data.data() + 56) = im_size;
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
        auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
            hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
    } /* deletes span, calls file.close(), deletes im_size */
    if (cache_items())
        LOG(std::format("cached {0} items from items.dat", items.size()));
    else std::cerr << "failed to cache items from items.dat" << std::endl, std::cin.ignore();

    ENetEvent event{};
    while (true) 
    {
        while (enet_host_service(server, &event, 10) > 0)
            std::jthread([&](std::stop_token stop)
	        {
                LOG(std::format("event.type = {0}", (int)event.type));
                switch (event.type) 
                {
                    case ENET_EVENT_TYPE_NONE: break;
				    case ENET_EVENT_TYPE_CONNECT:
                        if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[4]){0x1}, 4, ENET_PACKET_FLAG_RELIABLE)) < 0) break;
                        event.peer->data = new peer{};
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        delete getpeer;
                        break;
                    case ENET_EVENT_TYPE_RECEIVE: 
                    {
                        LOG(std::format("event.packet->data = {0}", std::span{event.packet->data, event.packet->dataLength}[0]));
                        std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                            std::string header{packet.begin() + 4, packet.end() - 1};
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                LOG(header);
                                std::call_once(getpeer->logging_in, [&]() 
                                {
                                    std::ranges::replace(header, '\n', '|'); /* e.g. requestedName|test\n = requestedName|test| */
                                    std::vector<std::string> read_once = readpipe(std::string{header}); 
                                    if (read_once[0] == "requestedName" or read_once[0] == "tankIDName") {
                                        read_once[0] == "requestedName" ? 
                                            getpeer->requestedName = read_once[1] + "_" + std::to_string(rand(100, 999)) :
                                            getpeer->tankIDName = read_once[1];
                                        if (not getpeer->tankIDName.empty() and getpeer->tankIDPass not_eq read_once[3])
                                        {
                                            gt_packet(*event.peer, 0, "OnConsoleMessage", "`4Unable to log on:`` That `wGrowID`` doesn't seem valid, or the password is wrong.  If you don't have one, press `wCancel``, un-check `w'I have a GrowID'``, then click `wConnect``.");
                                            enet_peer_disconnect_later(event.peer, 0);
                                            return;
                                        }
                                        short offset{};
                                        if (not getpeer->tankIDName.empty()) {
                                            getpeer->tankIDName = read_once[1];
                                            getpeer->tankIDPass = read_once[3];
                                            offset = 4;
                                        }
                                        getpeer->country = read_once[37 + offset];
                                    }
                                    gt_packet(*event.peer, 0,
                                        "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                        hash, 
                                        "ubistatic-a.akamaihd.net", 
                                        "0098/2521452/cache/", 
                                        "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                        "proto=208|choosemusic=audio/mp3/about_theme.mp3|active_holiday=12|wing_week_day=0|ubi_week_day=0|server_tick=59197218|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1|"
                                    );
                                });
                                if (header.starts_with("action|refresh_item_data"sv)) {
                                    enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_NO_ALLOCATE));
                                    break;
                                }
                                else if (header.starts_with("action|enter_game"sv))
                                {
                                    std::call_once(getpeer->entered_game, [&]() 
                                    {
                                        getpeer->user_id = peers().size();
                                        gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->requestedName).c_str());
                                        gt_packet(*event.peer, 0, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, getpeer->tankIDName.c_str(), getpeer->tankIDName.c_str());
                                        getpeer->slots.emplace_back(slot{18, 1});
                                        getpeer->slots.emplace_back(slot{32, 1});
                                        OnRequestWorldSelectMenu(event);
                                    });
                                }
                                break;

                            }
                            case 3: 
                            {
                                LOG(header);
                                if (header.contains("action|quit_to_exit"sv)) {
                                    peers([&](ENetPeer& p) {
                                        if (not getp->recent_worlds.empty() and not getpeer->recent_worlds.empty() and getp->recent_worlds.back() == getpeer->recent_worlds.back())
                                            gt_packet(p, 0, "OnRemove", std::format("netID|{}\n", getpeer->netid).c_str());
                                    });
                                    OnRequestWorldSelectMenu(event);
                                }
                                else if (header.contains("action|quit"sv)) 
                                    enet_peer_disconnect(event.peer, ENET_NORMAL_DISCONNECTION);
                                else if (header.starts_with("action|join_request"sv)) {
                                    std::ranges::replace(header, '\n', '|');
                                    std::string big_name{readpipe(std::string{header})[3]}; /* input: test -> TEST */
                                    std::ranges::transform(big_name, big_name.begin(), [](char c) { return std::toupper(c); });
                                    std::unique_ptr<world> w = read_world(big_name);
                                    if (w == nullptr) /* create a new world */
                                    {
                                        w = std::make_unique<world>(world{.name = big_name}); /* replace nullptr with world constructor */
                                        auto main_door = rand(2, ((100 * 60) / ((100 * 60) / 100) - 4));
                                        std::vector<block> blocks(100 * 60, block{0, 0});
                                        for (auto& b : blocks) {
                                            auto i = &b - &blocks[0];
                                            if (i >= 3700) {
                                                b.bg = 14; // cave background
                                                b.fg = (i >= 3800 and i < 5000 /* lava level */ and not rand(0, 38)) ? 10 : 
                                                    (i > 5000 and i < 5400 /* bedrock level */ and rand(0, 7) < 3) ? 4 : 
                                                    (i >= 5400) ? 8 : 2;
                                            }
                                            if (i == 3600 + main_door) b.fg = 6; // main door
                                            if (i == 3700 + main_door) b.fg = 8; // Bedrock below the door
                                        }
                                        w->blocks = std::move(blocks);
                                        write_world(w);
                                    }
                                    unsigned y = w->blocks.size() / 100, x = w->blocks.size() / y;
                                    std::vector<std::byte> data(78 + w->name.length() + w->blocks.size() + 24 + (8 * w->blocks.size()), std::byte{0x0});
                                    data[0] = std::byte{0x4};
                                    data[4] = std::byte{0x4};
                                    data[16] = std::byte{0x8};
                                    unsigned char name_size = w->name.length(); /* Growtopia limits world name length hence 255 is plenty of space */
                                    data[66] = std::byte{name_size};
                                    for (size_t i = 0; i < name_size; ++i)
                                        data[68 + i] = static_cast<std::byte>(w->name[i]);
                                    data[68 + name_size] = static_cast<std::byte>(x);
                                    data[72 + name_size] = static_cast<std::byte>(y);
                                    *reinterpret_cast<unsigned short*>(data.data() + 76 + name_size) = static_cast<unsigned short>(w->blocks.size());
                                    std::byte* pos = data.data() + 80 + name_size;
                                    std::array<short, 2> spawn_cord{};
                                    for (size_t i = 0; i < w->blocks.size(); ++i) {
                                        *reinterpret_cast<short*>(pos) = w->blocks[i].fg; pos += sizeof(short);
                                        *reinterpret_cast<short*>(pos + 2) = w->blocks[i].bg; pos += sizeof(short);
                                        *reinterpret_cast<unsigned*>(pos + 4) = w->blocks[i].flags; pos += sizeof(unsigned);
                                        /* TODO: fix main door visuals */
                                        if (w->blocks[i].fg == 6) {
                                            spawn_cord[0] = (i % x) * 32;
                                            spawn_cord[1] = (i / x) * 32;
                                            *(pos + 8) = std::byte{0x1}; pos += sizeof(std::byte);
                                            *reinterpret_cast<short*>(pos + 9) = sizeof("EXIT") - 1; pos += sizeof(short);
                                            for (size_t i = 0; i < sizeof("EXIT") - 1; ++i)
                                                *(pos + 11 + i) = static_cast<std::byte>(std::string_view{"EXIT"}[i]), pos += sizeof(char);
                                        }
                                    }
	                                enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    for (std::size_t i = 0; i < getpeer->recent_worlds.size() - 1; ++i)
                                        getpeer->recent_worlds[i] = getpeer->recent_worlds[i + 1];
                                    getpeer->recent_worlds.back() = w->name;
                                    gt_packet(*event.peer, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\ncolrect|0|0|20|30\nposXY|{2}|{3}\nname|{4}\ncountry|{5}\ninvis|0\nmstate|0\nsmstate|0\ntype|local\n",
                                        getpeer->netid, getpeer->user_id, spawn_cord[0], spawn_cord[1], getpeer->requestedName, getpeer->country).c_str());
                                    getpeer->netid = ++w->visitors;
                                    gt_packet(*event.peer, 0, "OnSetPos", std::vector<float>{static_cast<float>(spawn_cord[0]), static_cast<float>(spawn_cord[1])});
                                    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("World `w{0}`` entered.  There are `w{1}`` other people here, `w{2}`` online.",
                                        w->name, w->visitors - 1, peers().size()).c_str());
                                    inventory_visuals(*event.peer);
                                }
                                break;
                            }
                            case 4: 
                            {
                                std::unique_ptr<state> state{};
                                {
                                    std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x0});
                                    if ((packet.size() + 4) >= 60) { /* 52, 56... 56 + 4 = 60(?) */ // TODO: learn gtnoob logic (LOL)
                                        for (size_t i = 0; i < packet.size(); ++i)
                                            packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                        if (std::to_integer<unsigned char>(packet[12]) & 0x8 and packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) 
                                            packet.clear();
                                    }
                                    state = get_state(packet);
                                } /* deletes packet ahead of time */
                                LOG(std::format("state->type: {}", state->type));
                                switch (state->type) 
                                {
                                    case 0: /* movement */
                                    {
                                        LOG(std::format("{0}:{1}", state->pos[0], state->pos[1]));
                                        break;
                                    }
                                    case 3: break; /* placing blocks(?) */
                                    case 24: break; /* seems to happen before action|enter_game */
                                    default: break;
                                }
                                break;
                            }
                        }
                        enet_packet_destroy(event.packet); /* cleanup */
                        break;
                    }
                }
	        });
    }
    return 0;
}
