/* parent: peer.hpp */
#include <cstring> /* std::strlen() */

#include "include/enet.h"

union given {
    std::array<std::byte, 60> bytes;
    std::array<int, 4> buffer;
};

/*
@param peer to whom this packet is sent to.
@param wait_for prep the packet ahead of time and send it within the time provided. (milliseconds) e.g. 1000 = 1 second, 60000 = 1 minute
@param params list of param that structures your packet. each entry will be than identified as a const char* or signed/unsigned or float/double, 
                respectfully void* entires will not be accepted. e.g. classes, ptr, void
*/
template<typename... T>
void gt_packet(ENetEvent event, signed/*unsigned...*/ wait_for, T... params) {
	std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(61);
        std::fill(data.get(), data.get() + 61, std::byte{0x0});
        std::array<int, 5> buffer{0x4, 0x1, getpeer->netid, 0x8, wait_for};
        std::array<short, 5> offsets{0, 4, 8, 16, 24};
        for (size_t i = 0; i < buffer.size(); ++i) {
            for (size_t ii = 0; ii < sizeof(int); ++ii)
                (data.get() + offsets[i])[ii] = reinterpret_cast<const std::byte*>(&buffer[i])[ii];
        }
    size_t size = 61;
    std::byte index{0x0};
    std::apply([&](auto const&... param) {
        (..., (void)([&]()
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(param)>, const char*>) {
                LOG(std::format("String: {0}; Size: {1}", param, std::strlen(param)));
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(size + 2 + std::strlen(param) + sizeof(int));
                for (size_t i = 0; i < size; ++i)
                    this_data[i] = data[i];
                this_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x0, "the console message" -> 0x1 */
                this_data[size + 1] = std::byte{0x2};
                this_data[size + 2] = static_cast<std::byte>(std::strlen(param)); /* e.g. "OnConsoleMessage" -> 0x16 */
                this_data[size + 3] = this_data[size + 4] = this_data[size + 5] = std::byte{0x0};
                for (size_t i = 0; i < std::strlen(param); ++i) 
                    this_data[size + 6 + i] = static_cast<std::byte>(param[i]); /* e.g. set byte to each char. e.g. 'a' -> 0x97. 'z' = 0x122 */
                size = size + 2 + std::strlen(param) + sizeof(int);
                data = std::move(this_data);
            }
            else if constexpr (std::is_signed_v<std::decay_t<decltype(param)>> or std::is_unsigned_v<std::decay_t<decltype(param)>>) {
                LOG(std::format("Integer: {0}; Type: {1}", param, (std::is_signed_v<std::decay_t<decltype(param)>>) ? "signed" : "unsigned"));
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(size + 2 + sizeof(int));
                for (size_t i = 0; i < size; ++i)
                    this_data[i] = data[i];
                this_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x0, 43562/-43562 -> 0x1 */
                this_data[size + 1] = std::byte{(std::is_signed_v<std::decay_t<decltype(param)>>) ? 0x9 : 0x5}; 
                for (size_t i = 0; i < sizeof(param); ++i)
                    this_data[size + 2 + i] = reinterpret_cast<std::byte const*>(&param)[i];
                size = size + 2 + sizeof(int);
                data = std::move(this_data);
            }
            index = static_cast<std::byte>(std::to_integer<int>(index) + 1);
		    if (size >= 61) 
                data[60] = static_cast<std::byte>(index);
        }()
    ));
    }, std::make_tuple(params...));
        ENetPacket* packet = enet_packet_create(data.get(), size, ENET_PACKET_FLAG_RELIABLE);
	        if (enet_peer_send(event.peer, 0, packet) == 0) 
                LOG(std::format("Released Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), size));
            else LOG(std::format("Invalid Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), size) );
    
};
