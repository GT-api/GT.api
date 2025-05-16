/*
    @copyright gurotopia (c) 25-6-2024
    @author @leeendl | Lead Contributor, English Comments

    Project has open arms for contribution!

    looking for:
    - Indonesian translator
*/
#include "include/pch.hpp" // @todo #pragma once
#include "include/database/items.hpp" // @note items.dat reading
#include "include/network/compress.hpp" // @note isalzman's compressor
#include "include/database/peer.hpp" // @note peer class
#include "include/event_type/event_type"

#include <fstream>
#include <future>

int main()
{
    {
        ENetCallbacks callbacks{
            .malloc = &malloc,
            .free = &free,
            .no_memory = []() { printf("\e[1;31mENet memory overflow\e[0m\n"); }
        };
        enet_initialize_with_callbacks(ENET_VERSION, &callbacks);
        printf("\e[38;5;247mENet initialize success! (v%d.%d.%d)\e[0m\n", ENET_VERSION_MAJOR, ENET_VERSION_MINOR, ENET_VERSION_PATCH);
    } // @note delete callbacks
    server = enet_host_create({
        .host = in6addr_any,
        .port = 17091
    },
    ENET_PROTOCOL_MAXIMUM_PEER_ID, 2);
    
    server->checksum = enet_crc32;
    enet_host_compress_with_range_coder(server);
    {
        std::ifstream file("items.dat", std::ios::binary | std::ios::ate);
        if (!file.is_open()) printf("\e[1;31mfailed to open items.dat\e[0m\n");
        std::streampos size = file.tellg();
        im_data.resize(im_data.size() + size); // @note state + items.dat
        im_data[0] = std::byte{ 04 }; // @note 04 00 00 00 packet flag
        im_data[4] = std::byte{ 0x10 }; // @note 16 00 00 00 (state::type)
        // @note {8}
        im_data[16] = std::byte{ 0x08 }; // @note 08 00 00 00 (state::id)
        *reinterpret_cast<std::streampos*>(&im_data[56]) = size; // @note 4 bits (items.dat size)
        file
            .seekg(0, std::ios::beg) // @note start from beginning of items.dat
            .read(reinterpret_cast<char*>(&im_data[60]), size); // @note 04 00 00 00 16 00 00 00 {8} 08 00 00 00 {4} {items.dat}
    } // @note delete file, size and closes file
    cache_items();

    ENetEvent event{};
    std::vector<std::thread> threads{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            if (const auto i = event_pool.find(event.type); i != event_pool.end())
                threads.emplace_back([=] { i->second(event); }).join(); // @todo
    return 0;
}
