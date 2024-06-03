#include <iostream>
#include <fstream>
#include <format> // enhanced string arguements & better logging
#include <thread> /* std::jthread() & std::chrono / std::this_thread */
#include <vector>
#include <numeric> // accumulate for items.dat hash (acc for short)

using namespace std::literals;

#include "include/enet.h"

#include "peer.hpp"
#include "packet.hpp"

int main() {
    /* TODO: implement red coloring for errors. */
    if (std::endian::native == std::endian::big) 
        std::cerr << "this project was made of understanding bytes, furthermore this project does not support endian::big" << std::endl, std::cin.ignore();
    if (enet_initialize() not_eq 0) 
        std::cerr << "enet_initialize() failed" << std::endl, std::cin.ignore();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};
        
    ENetHost* server = enet_host_create(&address, 100, 1, 0, 0);
        server->checksum = enet_crc32;
        if (enet_host_compress_with_range_coder(server) < 0 or server->checksum == (ENetChecksumCallback)0xFFFFFFFF) [[unlikely]]
            std::cerr << "failed to initialize packet compressor" << std::endl, std::cin.ignore();

    std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
        std::streamsize im_size = file.tellg();
        std::vector<std::byte> im_data(im_size + 60);
            std::fill(im_data.begin(), im_data.begin() + 60, std::byte{0});
            std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
                for (char i = 0; i < buffer.size(); ++i) 
                    std::copy(reinterpret_cast<std::byte*>(&buffer[i]), reinterpret_cast<std::byte*>(&buffer[i]) + sizeof(buffer[i]), im_data.begin() + i * sizeof(int));
            std::copy(reinterpret_cast<std::byte*>(&im_size), reinterpret_cast<std::byte*>(&im_size) + sizeof(im_size), im_data.begin() + 56);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
            auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
                signed hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
                LOG(std::format("items.dat hash: {0}", hash));
    file.close();

    ENetEvent event{};
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (enet_host_service(server, &event, 1000) > 0)
            std::jthread([&](std::stop_token stop)
	        {
                LOG(std::format("event.type = {0}", (int)event.type));
                switch (event.type) 
                {
				    case ENET_EVENT_TYPE_CONNECT: 
                    {
                        ENetPacket* packet = enet_packet_create(nullptr, sizeof(int), ENET_PACKET_FLAG_RELIABLE);
                        packet->data[sizeof(int) - 4] = static_cast<char>(0x1);
                        for (int i = 1; i < sizeof(int); ++i) 
                            packet->data[sizeof(int) - 4 + i] = static_cast<char>(0x0);
                        enet_peer_send(event.peer, 0, packet); /* 0x1 0x0 0x0 0x0 */
                        event.peer->data = new peer{};
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: 
                    {
                        delete getpeer;
                        event.peer->data = nullptr;
                        break;
                    }
                    case ENET_EVENT_TYPE_RECEIVE: 
                    {
                        LOG(std::format("event.packet->data = {0}", std::span{event.packet->data, event.packet->dataLength}[0]));
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                                    std::string_view header{packet.begin() + 4, packet.end() - 1};
                                LOG(header);
                                if (not getpeer->LoginLoop) /* investigating this */
                                {
                                    getpeer->LoginLoop = true;
                                    gt_packet(event, 0,
                                        "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                        hash, 
                                        "ubistatic-a.akamaihd.net", 
                                        "0098/2805202400/cache/", 
                                        "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                        "proto=207|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=5231383|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1"
                                    );
                                }
                                /* TODO */
                                if (header.starts_with("action|refresh_item_data"sv))
                                {
							        enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_RELIABLE));
                                    enet_peer_disconnect_later(event.peer, 0);
                                    break;
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
