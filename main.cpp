/*
    @copyright gurotopia (c) 25-6-2024
    @author leeendl | English Comments

    Project has open arms for contribution!
*/
#include "include\pch.hpp"
#include "include\database\items.hpp" // @note items.dat reading
#include "include\network\compress.hpp" // @note isalzman's compressor
#include "include\database\peer.hpp" // @note everything relating to the peer
#include "include\event_type\event_type"

#include <fstream>
#include <future>

int main()
{
    {
        ENetCallbacks callbacks{
            .malloc = &malloc, 
            .free = &free, 
            .no_memory = []() { printf("enet memory overflow"); }
        };
        enet_initialize_with_callbacks(ENET_VERSION, &callbacks);
        printf("ENet initialize success! (using ENet v%d.%d.%d)\n", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
    } // @note delete callbacks
    server = enet_host_create({
        .host = in6addr_any, 
        .port = 17091
    }, 
    ENET_PROTOCOL_MAXIMUM_PEER_ID, 2);

    server->checksum = enet_crc32;
    enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary bitor std::ios::ate);
        if (not file.is_open()) {
            printf("failed to open items.dat");
            getchar();
        }
        std::streampos size = file.tellg();
        im_data.resize(im_data.size() + size);
        im_data[0] = std::byte{ 04};
        im_data[4] = std::byte{ 0x10 };
        *reinterpret_cast<int*>(&im_data[8]) = -1; // @note ff ff ff ff
        im_data[16] = std::byte{ 0x08 };
        *reinterpret_cast<std::streampos*>(&im_data[56]) = size;
        file
            .seekg(0, std::ios::beg) // @note  start from beginning of items.dat
            .read(reinterpret_cast<char*>(&im_data[60]), size);
    } // @note delete file, size and closes file
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            if (const auto i = event_pool.find(event.type); i not_eq event_pool.end())
                threads.emplace_back([=] {
                    srand(time(nullptr) ^ std::hash<std::thread::id>{}(std::this_thread::get_id()));
                    i->second(event);
                }).detach();
    return 0;
}
