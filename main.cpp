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

#define ENET_IMPLEMENTATION
#include "include/enet.hpp"

#include "items.hpp"
#include "include/sqlite3.hpp" /* for storing peer, world data */
#include "peer.hpp"
#include "packet.hpp"
#include "world.hpp"
#include "include/compress.hpp" /* for growtopia packet (byte) compressor; a must for a 'GTPS' */

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

int main() {
    if (enet_initialize() not_eq 0) 
        std::cerr << "enet_initialize() failed" << std::endl, std::cin.ignore();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};
    
    server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 0, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
            if (not file) std::cout << "failed to open items.dat" << std::endl, std::cin.ignore();
        std::streamsize im_size = file.tellg();
        im_data.resize(im_size + 60);
        std::fill(im_data.begin(), im_data.begin() + 60, std::byte{0x0});
        std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
             for (size_t i = 0; i < buffer.size(); ++i) 
                 new (&im_data[i * sizeof(int)]) int(buffer[i]);
         new (&im_data[56]) size_t(im_size);
         file.seekg(0, std::ios::beg);
         file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
          file.close();
          auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
              hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                  [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
             LOG(std::format("items.dat hash: {0}", hash));
    }
    if (cache_items())
        LOG(std::format("cached {0} items from items.dat", items.size()));
    else LOG("failed to cache items from items.dat");

    ENetEvent event{};
    while (true) 
    {
        while (enet_host_service(server, &event, 1000) > 0)
            std::jthread([&](std::stop_token stop)
	        {
                LOG(std::format("event.type = {0}", (int)event.type));
                switch (event.type) 
                {
                    case ENET_EVENT_TYPE_NONE: break;
				    case ENET_EVENT_TYPE_CONNECT:
                        if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[4]){0x1, 0x0, 0x0, 0x0}, 4, ENET_PACKET_FLAG_RELIABLE)) < 0) break;
                        event.peer->data = new peer{};
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        delete getpeer;
                        event.peer->data = nullptr;
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
                                    getpeer->user_id = peers().size();
                                    std::ranges::replace(header, '\n', '|');
                                    if (readpipe(std::string{header})[0] == "requestedName")
                                        getpeer->requestedName = readpipe(std::string{header})[1] + "_" + std::to_string(rand(1000, 9999)); 
                                    else {
                                        getpeer->tankIDName = readpipe(std::string{header})[1];
                                        getpeer->tankIDPass = readpipe(std::string{header})[3];
                                    }
                                    short offset{};
                                    if (readpipe(std::string{header})[0] == "tankIDName") offset = 4;
                                    getpeer->country = readpipe(std::string{header})[37 + offset];
                                    gt_packet(*event.peer, 0,
                                        "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                        hash, 
                                        "ubistatic-a.akamaihd.net", 
                                        "0098/2805202400/cache/", 
                                        "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                        "proto=207|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=5231383|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1"
                                    );
                                });
                                if (header.starts_with("action|refresh_item_data"sv))
                                {
                                    enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    break;
                                }
                                else if (header.starts_with("action|enter_game"sv))
                                {
                                    read_peer(*event.peer); /* load peer's previous session data. */
                                    getpeer->visual_name = getpeer->tankIDName.empty() ? getpeer->requestedName : getpeer->tankIDName;

                                    gt_packet(*event.peer, 0, "OnFtueButtonDataSet", 0, 0, 0, "|||||");
                                    gt_packet(*event.peer, 0, "SetHasGrowID", getpeer->tankIDName.empty() ? 0 : 1, std::string{getpeer->tankIDName}.c_str(), std::string{getpeer->tankIDName}.c_str());
                                    gt_packet(*event.peer, 0, "OnRequestWorldSelectMenu", "add_filter|\nadd_heading|Top Worlds<ROW2>|");
                                    gt_packet(*event.peer, 0, "OnConsoleMessage", std::format("Where would you like to go? (`w{}`` online)", peers().size()).c_str());
                                    break;
                                }
                                break;
                            }
                            case 3: 
                            {
                                if (header.starts_with("action|join_request"sv)) {
                                    std::ranges::replace(header, '\n', '|');
                                    auto w = std::make_unique<world>();
                                    w->name = readpipe(std::string{header})[3];
                                    int d_ = rand(2, (6000 / (6000 / 100) - 4));
	                                for (int i = 0; i < 6000; i++) {
	                                	block b{ 0, 0, uint16_t(-1) };
	                                	if (i >= 3800 && i < 5400 && !(rand() % 50)) b.fg = 10;
	                                	else if (i >= 3700 && i < 5400) 
	                                		if (i > 5000) 
	                                			if (rand() % 8 < 3) b.fg = 4; 
	                                			else b.fg = 2; 
	                                		else b.fg = 2;
	                                	else if (i >= 5400) b.fg = 8;
	                                	if (i == 3600 + d_) b.fg = 6;
	                                	if (i == 3700 + d_) b.fg = 8;
	                                	if (i >= 3700) b.bg = 14;
	                                	w->blocks.push_back(b);
	                                }

                                    unsigned ySize = w->blocks.size() / 100, xSize = w->blocks.size() / ySize, square = w->blocks.size();
                                    int alloc = (8 * square) + (w->floating.size() * 16);
	                                std::vector<BYTE> data(78 + w->name.length() + square + 24 + alloc, 0);
	                                data[0] = static_cast<BYTE>(0x4);
                                    data[4] = static_cast<BYTE>(0x4);
                                    data[16] = static_cast<BYTE>(0x8);

                                    size_t name_size = w->name.length();
                                    data[66] = static_cast<BYTE>(name_size);
                                    std::memcpy(data.data() + 68, w->name.c_str(), name_size);
                                    data[68 + name_size] = static_cast<BYTE>(xSize);
                                    data[72 + name_size] = static_cast<BYTE>(ySize);
                                    *reinterpret_cast<unsigned short*>(data.data() + 76 + name_size) = static_cast<unsigned short>(square);
	                                std::span<BYTE> blc = std::span(data).subspan(80 + name_size);
	                                for (int i = 0; i < square; ++i) {
    	                                auto block_span = blc.subspan(i * 8, 8);
    	                                *reinterpret_cast<unsigned short*>(block_span.data()) = w->blocks[i].fg;
    	                                *reinterpret_cast<unsigned short*>(block_span.data() + 2) = w->blocks[i].bg;
    	                                *reinterpret_cast<unsigned*>(block_span.data() + 4) = w->blocks[i].flags;
	                                }
	                                int count = static_cast<int>(w->floating.size()), last_uid = count;
	                                *reinterpret_cast<int*>(blc.subspan(square * 8, 4).data()) = count;
	                                *reinterpret_cast<int*>(blc.subspan(square * 8 + 4, 4).data()) = last_uid;
                                    for (const auto& floating : w->floating) {
                                        *reinterpret_cast<uint16_t*>(blc.data()) = static_cast<uint16_t>(floating.id);
                                        *reinterpret_cast<float*>(blc.data() + 2) = floating.x;
                                        *reinterpret_cast<float*>(blc.data() + 6) = floating.y;
                                        *reinterpret_cast<uint16_t*>(blc.data() + 10) = static_cast<uint16_t>(floating.count);
                                        *reinterpret_cast<uint32_t*>(blc.data() + 12) = floating.uid;
                                        blc = blc.subspan(16);
                                    }
	                                enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    gt_packet(*event.peer, 0, "OnSetBux", 0, 1, 0);
                                    ++w->visitors;
                                    getpeer->netid = w->visitors;
                                    gt_packet(*event.peer, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\neid|\nip|\ncolrect|0|0|20|30\nposXY|1440|736\nname|{2}\ntitleIcon|\ncountry|{3}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|",
                                        getpeer->netid, getpeer->user_id, getpeer->tankIDName.empty() ? getpeer->requestedName : getpeer->tankIDName, std::string{getpeer->country}.c_str()));
                                    gt_packet(*event.peer, 0, "OnSpawn", std::format(
                                        "spawn|avatar\nnetID|{0}\nuserID|{1}\neid|\nip|\ncolrect|0|0|20|30\nposXY|1440|736\nname|{2}\ntitleIcon|\ncountry|{3}\ninvis|0\nmstate|0\nsmstate|0\nonlineID|\ntype|local",
                                        getpeer->netid, getpeer->user_id, getpeer->tankIDName.empty() ? getpeer->requestedName : getpeer->tankIDName, std::string{getpeer->country}.c_str()));
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
