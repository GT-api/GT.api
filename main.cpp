#include <iostream>
#include <fstream>
#include <format> // enhanced string arguements & better logging
#include <thread> /* std::jthread() & std::chrono / std::this_thread */
#include <vector>
#include <numeric> // accumulate for items.dat hash (acc for short)

#define ENET_IMPLEMENTATION
#include "include/enet.hpp"

#include "peer.hpp"
#include "packet.hpp"
#include "include/compress.hpp"

using namespace std::literals;

int main() {
    if (enet_initialize() not_eq 0) 
        std::cerr << "enet_initialize() failed" << std::endl, std::cin.ignore();

    ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};
    
    ENetHost* server = enet_host_create(&address, 100, 0, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);

    std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
        std::streamsize im_size = file.tellg();
            if (im_size == -1) std::cerr << "failed to read items.dat in binary mode" << std::endl, std::cin.ignore();
        std::vector<std::byte> im_data(im_size + 60);
            std::fill(im_data.begin(), im_data.begin() + 60, std::byte{0x0});
            std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
                for (size_t i = 0; i < buffer.size(); ++i) 
                    new (&im_data[i * sizeof(int)]) int(buffer[i]);
            new (&im_data[56]) size_t(im_size);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(im_data.data() + 60), im_size);
            auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
                signed hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
                LOG(std::format("items.dat hash: {0}", hash));
    file.close();

    ENetEvent event{};
    while (true) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (enet_host_service(server, &event, 1000) > 0)
            std::jthread([&](std::stop_token stop)
	        {
                LOG(std::format("event.type = {0}", (int)event.type));
                switch (event.type) 
                {
                    case ENET_EVENT_TYPE_NONE: break;
				    case ENET_EVENT_TYPE_CONNECT: 
                    {
                        ENetPacket* packet = enet_packet_create(nullptr, 4, ENET_PACKET_FLAG_RELIABLE);
                        packet->data[0] = enet_uint8{0x1};
                        packet->data[1] = packet->data[2] = packet->data[3] = enet_uint8{0x0};
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
                        std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                            std::string_view header{packet.begin() + 4, packet.end() - 1};
                            LOG(header);
                        switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                        {
                            case 2: 
                            {
                                std::call_once(getpeer->logging_in, [&](){
                                    gt_packet(event, 0,
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
                                    gt_packet(event, 0, "OnRequestWorldSelectMenu"); /* TODO: add categories, recent, popular world buttons */
                                    break;
                                }
                                break;
                            } /* case 2: */
                            case 3: 
                            {
                                if (header.starts_with("action|quit_to_exit"sv)); /* TODO: implement player leaving world */
                                else if (header.starts_with("action|join_request"sv)); /* TODO: implement player entering world */
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
