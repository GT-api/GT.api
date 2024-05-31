#include <iostream>
#include <fstream>
#include <format> // enhanced string arguements & better logging

#include <thread>

#include <vector>
#include <span> // ref array object (for vector)

#include <numeric> // accumulate for items.dat hash (acc for short)

#include <enet/win32.c>
#include <enet/list.c>
#include <enet/protocol.c>
#include <enet/peer.c>
#include <enet/callbacks.c>
#include <enet/host.c>

#include <enet/compress.c> // TODO

#include "peer.hpp"
#include "packet.hpp"

template<typename T>
void shift_pos(std::vector<std::byte>& data, short& pos, T& value) {
    for (short i = 0; i < sizeof(T); ++i) 
        reinterpret_cast<std::byte*>(&value)[i] = data[pos + i];
    pos += sizeof(T);
}

int main() {
    if (enet_initialize() not_eq 0) std::cerr << "enet_initialize() failed" << std::endl;

    ENetAddress address{}; 
        enet_address_set_host(&address, "0.0.0.0");
        address.port = 17091;
        
    ENetHost* server = enet_host_create(&address, 100, 1, 0, 0);
        server->checksum = enet_crc32;
	    enet_host_compress_with_range_coder(server);

    std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        std::vector<std::byte> data(size + 60);
            std::fill(data.begin(), data.begin() + 60, std::byte{0});
            std::array<int, 4> buffer{0x4, 0x10, -1, 0x8};
                for (char i = 0; i < buffer.size(); ++i) 
                    std::copy(reinterpret_cast<std::byte*>(&buffer[i]), reinterpret_cast<std::byte*>(&buffer[i]) + sizeof(buffer[i]), data.begin() + i * 4);
            std::copy(reinterpret_cast<std::byte*>(&size), reinterpret_cast<std::byte*>(&size) + sizeof(size), data.begin() + 56);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(data.data() + 60), size);
            auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(data.data()), data.size());
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
                        ENetPacket* const packet = enet_packet_create(nullptr, 5, ENET_PACKET_FLAG_RELIABLE);
                            *reinterpret_cast<int*>(packet->data) = 0x1;
                            enet_peer_send(event.peer, 0, packet);
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
                        LOG(std::format("event.packet->data = {0}", (int)*(event.packet->data)));
                        if (event.packet->dataLength < 3u) break;
                        switch (*(event.packet->data)) {
                            case 2: 
                            {
                                gt_packet(event.peer, 
                                    "OnSuperMainStartAcceptLogonHrdxs47254722215a", 
                                    hash, 
                                    "ubistatic-a.akamaihd.net", 
                                    "0098/2805202400/cache/", 
                                    "cc.cz.madkite.freedom org.aqua.gg idv.aqua.bulldog com.cih.gamecih2 com.cih.gamecih com.cih.game_cih cn.maocai.gamekiller com.gmd.speedtime org.dax.attack com.x0.strai.frep com.x0.strai.free org.cheatengine.cegui org.sbtools.gamehack com.skgames.traffikrider org.sbtoods.gamehaca com.skype.ralder org.cheatengine.cegui.xx.multi1458919170111 com.prohiro.macro me.autotouch.autotouch com.cygery.repetitouch.free com.cygery.repetitouch.pro com.proziro.zacro com.slash.gamebuster", 
                                    "proto=207|choosemusic=audio/mp3/about_theme.mp3|active_holiday=0|wing_week_day=0|ubi_week_day=0|server_tick=5231383|clash_active=0|drop_lavacheck_faster=1|isPayingUser=0|usingStoreNavigation=1|enableInventoryTab=1|bigBackpack=1"
                                );
                            }
                                break;
                            }
                        break;
                    }
                }
	        });
    }
    return 0;
}
