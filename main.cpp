/*
    GT.api (c) 2024
    Project has open arms for contribution (friendly, no stress)
    @author leeendl | Discord: leeendl
*/
#include "include\database\items.hpp" // @note items.dat reading
#include "include\network\enet.hpp" // @note ENet supporting AF_INET6
#include "include\database\peer.hpp" // @note everything relating to the peer
#include "include\network\packet.hpp" // @note back-end packet dealing (using ENet & basic C++ concepts)
#include "include\tools\string_view.hpp" // @note stuff that isn't in the standard C++ library
#include "include\database\world.hpp" // @note everything related to a world
#include "include\tools\random_engine.hpp" // @note optimized random number generator

#include "include\on\on"
#include "include\action\actions"
#include "include\state\states"
#include "include\network\jtpool.hpp"
#include "include\event_type\event_type"

void git_check(const std::string& commit); // -> import git.o
void basic_https(const std::string& s_ip, u_short s_port, u_short https_port); // -> import https.o
int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o -- Open Source: https://github.com/lsalzman/enet/blob/master/compress.c

int main() 
{
    git_check("9ad295fb65efc7282f6bd77d0bb057cf20051c2c");
    enet_initialize();
    server = enet_host_create({.host = in6addr_any, .port = 17091}, ENET_PROTOCOL_MAXIMUM_PEER_ID, 1, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    std::thread(&basic_https, "127.0.0.1", server->address.port, 443).detach();
    {std::ifstream file("items.dat", std::ios::binary bitor std::ios::ate);
        std::streamsize size = file.tellg();
        im_data.resize(size + 60);
        for (int i = 0; i < 5; ++i)
            *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) = std::array<int, 5>{4, 16, -1, 0, 8}[i];
        *reinterpret_cast<int*>(im_data.data() + 56) = size;
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(im_data.data() + 60), size);} // @note delete & close file
    cache_items();

    ENetEvent event{}; // @todo
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            if (auto i = event_pool.find(event.type); i not_eq event_pool.end())
                jt_handler.enqueue(3, [=] { i->second(event); });
    return 0;
}
