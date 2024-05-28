#include <iostream>
#include <fstream>
#include <format> // enhanced string arguements & better logging

#include <thread>

#include <vector>
#include <span> // ref array object (for vector)

#include <numeric> // accumulate for items.dat hash (acc for short)

#include <enet/win32.c>
#include <enet/list.c>
#include <enet/packet.c>
#include <enet/protocol.c>
#include <enet/peer.c>
#include <enet/callbacks.c>
#include <enet/host.c>

#include <enet/compress.c> // TODO

using pr = std::vector<int>;

template<typename T>
void shift_pos(std::vector<std::byte>& buffer, short& pos, T& value) {
    if (pos >= 0x7fff) return;
    std::copy(buffer.begin() + pos, buffer.begin() + pos + sizeof(T), reinterpret_cast<std::byte*>(&value));
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
        std::vector<std::byte> buffer(size + 60);
            std::fill(buffer.begin(), buffer.begin() + 60, std::byte{0});
            pr info{0x4, 0x10, -1, 0x8};
                for (char i = 0; i < info.size(); ++i) 
                    std::copy(reinterpret_cast<std::byte*>(&info[i]), reinterpret_cast<std::byte*>(&info[i]) + sizeof(info[i]), buffer.begin() + i * 4);
            std::copy(reinterpret_cast<std::byte*>(&size), reinterpret_cast<std::byte*>(&size) + sizeof(size), buffer.begin() + 56);
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(buffer.data() + 60), size);
            auto span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size());
                int hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });

        short pos{60}, count{0}, version{0};
        shift_pos(buffer, pos, version);
        shift_pos(buffer, pos, count);
        for (short i = 0; i < count; i++) {

        }
    
    file.close();

    ENetEvent event{};
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        while (enet_host_service(server, &event, 1000) > 0)
            std::jthread([&](std::stop_token handle)
	        {
                std::clog << std::format("event.type = {0}", (int)event.type) << std::endl;
                switch (event.type) 
                {
				    case ENET_EVENT_TYPE_CONNECT: 
                    {
                        std::vector<int> buffer{1, 0, 0};
                        const auto packet = enet_packet_create(nullptr, buffer[1] + 5, ENET_PACKET_FLAG_RELIABLE);
                        std::copy(reinterpret_cast<std::byte*>(&buffer[0]), reinterpret_cast<std::byte*>(&buffer[0]) + 4, packet->data);
                        std::copy(reinterpret_cast<std::byte*>(&buffer[2]), reinterpret_cast<std::byte*>(&buffer[2]) + 1, packet->data + 2 + buffer[1]);
                        enet_peer_send(event.peer, 0, packet);
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: 
                    {
                        break;
                    }
                }
	        });
    }
    return 0;
}