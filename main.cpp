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
    github::sync("cf0846de6f4920b295c161d65e9f0a70a01378ec");
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
        std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
        memcpy(im_data.data(), &buffer[0], sizeof(buffer[0]));
	    memcpy(im_data.data() + 4, &buffer[1], sizeof(buffer[1]));
	    memcpy(im_data.data() + 8, &buffer[2], sizeof(buffer[2]));
	    memcpy(im_data.data() + 16, &buffer[3], sizeof(buffer[3]));
	    memcpy(im_data.data() + 56, &im_size, 4);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
        file.close();
        auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
            hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
            LOG(std::format("items.dat hash: {}", hash));
    }
    if (cache_items())
        LOG(std::format("cached {0} items from items.dat", items.size()));
    else LOG("failed to cache items from items.dat");

    ENetEvent event{};
    while (true) 
    {
        while (enet_host_service(server, &event, 0) > 0)
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
                        write_peer(*event.peer); /* TODO promise */
                        delete getpeer;
                        break;
                    case ENET_EVENT_TYPE_RECEIVE: 
                    {
                        LOG(std::format("event.packet->data = {0}", std::span{event.packet->data, event.packet->dataLength}[0]));
                        std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                            std::string header{packet.begin() + 4, packet.end() - 1};
                            LOG(header);
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                std::call_once(getpeer->logging_in, [&]() 
                                {
                                    std::ranges::replace(header, '\n', '|'); /* e.g. requestedName|test\n = requestedName|test| */
                                    std::vector<std::string> read_once = readpipe(std::string{header}); 
                                    if (read_once[0] == "requestedName" or read_once[0] == "tankIDName") {
                                        read_once[0] == "requestedName" ? 
                                            getpeer->requestedName = read_once[1] + "_" + std::to_string(rand(100, 999)) :
                                            getpeer->tankIDName = read_once[1];
                                        if (not getpeer->tankIDName.empty())
                                            read_peer(*event.peer);
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
                                        write_peer(*event.peer);
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
                                        getpeer->visual_name = getpeer->tankIDName.empty() ? getpeer->requestedName : getpeer->tankIDName;

                                        gt_packet(*event.peer, 0, "OnFtueButtonDataSet", 0, 0, 0, "||0|||-1", "", "1|1");
                                        gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Welcome back, `w`w{}````.", getpeer->visual_name).c_str());
                                        gt_packet(*event.peer, 0, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, std::string{getpeer->tankIDName}.c_str(), std::string{getpeer->tankIDName}.c_str());
                                        OnRequestWorldSelectMenu(event);
                                    });
                                }
                                break;

                            }
                            case 3: 
                            {
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
                                    auto w = std::make_unique<world>();
                                    w->name = readpipe(std::string{header})[3];
                                    std::ranges::transform(w->name, w->name.begin(), [](char c) { return std::toupper(c); });
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
                                    w->blocks = std::move(blocks); // Move the vector to the destination
                                    unsigned y = w->blocks.size() / 100, x = w->blocks.size() / y;
                                    int alloc = (8 * w->blocks.size()) + (w->floating.size() * 16), s1 = 4, s3 = 8;
                                    std::vector<std::byte> data(78 + w->name.length() + w->blocks.size() + 24 + alloc, std::byte{0x0});
                                    data[0] = std::byte{0x4};
                                    data[4] = std::byte{0x4};
                                    data[16] = std::byte{0x8};
                                    size_t name_size = w->name.length();
                                    data[66] = std::byte{name_size};
                                    for (size_t i = 0; i < name_size; ++i)
                                        data[68 + i] = static_cast<std::byte>(w->name[i]);
                                    data[68 + name_size] = std::byte{x};
                                    data[72 + name_size] = std::byte{y};
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
                                    getpeer->netid = ++w->visitors;
                                    for (std::size_t i = 0; i < getpeer->recent_worlds.size() - 1; ++i)
                                        getpeer->recent_worlds[i] = getpeer->recent_worlds[i + 1];
                                    getpeer->recent_worlds.back() = w->name;
                                    gt_packet(*event.peer, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\ncolrect|0|0|20|30\nposXY|{2}|{3}\nname|{4}\ncountry|{5}\ninvis|0\nmstate|0\nsmstate|0\ntype|local",
                                        getpeer->netid, getpeer->user_id, spawn_cord[0], spawn_cord[1], getpeer->tankIDName.empty() ? getpeer->requestedName : getpeer->tankIDName, getpeer->country).c_str());
                                    gt_packet(*event.peer, 0, "OnSetClothing", 
                                        std::vector<float>{0, 0, 0},
                                        std::vector<float>{0, 0, 0},
                                        std::vector<float>{0, 0, 0}, 0/*get real GT normal skin color*/,
                                        std::vector<float>{0, 0, 0});
                                    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("World `w{0}`` entered.  There are `w{1}`` other people here, `w{2}`` online.",
                                        w->name, w->visitors - 1, peers().size()));
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
	        });
    }
    return 0;
}
