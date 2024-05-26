#include <iostream>
#include <thread>

#include <enet/win32.c>

int main() {
    if (enet_initialize() not_eq 0) std::cerr << "enet_initialize() failed" << std::endl;

    ENetAddress address{}; 
        enet_address_set_host(&address, "0.0.0.0");
        address.port = 17091;
        
    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0);
        server->checksum = enet_crc32;
		enet_host_compress_with_range_coder(server);
    
    ENetEvent event{};
        while (true) {
            // some thread. idk yet
        }
    return 0;
}