/*
    @copyright GT.api (c) 2024
    @author leeendl | English Comments

    Project has open arms for contribution!
*/
#include "include\database\items.hpp" // @note items.dat reading
#include "include\network\enet.hpp" // @note ENet supporting AF_INET6
#include "include\database\peer.hpp" // @note everything relating to the peer
#include "include\network\packet.hpp" // @note back-end packet dealing (using ENet & basic C++ concepts)
#include "include\database\world.hpp" // @note everything related to a world

#include "include\on\on"
#include "include\commands\commands"
#include "include\tools\string_view.hpp" // @note read '|' in strings & check if string uses standard char(s)
#include "include\action\actions"
#include "include\state\states"
#include "include\event_type\event_type"

int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o -- Open Source: https://github.com/lsalzman/enet/blob/master/compress.c

int main()
{
    {
        ENetCallbacks callbacks{
            .malloc = &malloc, 
            .free = &free, 
            .no_memory = []() { printf("enet memory overflow"); }
        };
        enet_initialize_with_callbacks(ENET_VERSION, &callbacks);
    }
    server = enet_host_create({
        .host = in6addr_any, 
        .port = 17091}, 
        ENET_PROTOCOL_MAXIMUM_PEER_ID, 2);

    server->checksum = enet_crc32;
    enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary bitor std::ios::ate);
        if (not file.is_open()) {
            printf("failed to open items.dat");
            getchar();
        }
        std::streampos size = file.tellg(); // @note size of ios::ate (end of file). this is called before seekg (ios::beg (beginning of file)).
        im_data.resize(im_data.size() + size);
        *reinterpret_cast<std::array<unsigned char, 20>*>(&im_data[0]) = {
            0x4, 0x0, 0x0, 0x0, 
            0x10, 0x0, 0x0, 0x0, 
            0xFF, 0xFF, 0xFF, 0xFF, 
            0x0, 0x0, 0x0, 0x0, 
            0x8, 0x0, 0x0, 0x0
        };
        *reinterpret_cast<std::streampos*>(&im_data[20 + 36]) = size;
        file
            .seekg(0, std::ios::beg) // @note  start from beginning of items.dat
            .read(reinterpret_cast<char*>(&im_data[60]), size);
    } // @note delete & close file
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            if (auto i = event_pool.find(event.type); i not_eq event_pool.end())
                threads.emplace_back([=] { 
                    srand(static_cast<unsigned int>(time(0)) ^ std::hash<std::thread::id>{}(std::this_thread::get_id()));
                    i->second(event); 
                }).detach();
    return 0;
}
