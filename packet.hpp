#include <enet/packet.c>
#include <array>
#include <numeric>
#include <memory>
#include <vector> 
#include <cstring>
#include <algorithm>
#include <type_traits>

/*
@param peer to whom this packet is sent to.
@param wait_for prep the packet ahead of time and send it within the time provided. (milliseconds) e.g. 1000 = 1 second, 60000 = 1 minute
@param params list of param that structures your packet. each entry will be than identified as a const char* or signed/unsigned or float/double, 
                respectfully void* entires will not be accepted. e.g. classes, ptr, void
*/
template<typename... T>
void gt_packet(ENetEvent event, signed/*unsigned...*/ wait_for, T... params) {
	std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(61);
        std::fill_n(data.get(), 61, std::byte{0x0});
        std::array<int, 5> buffer{0x4, 0x1, getpeer->netid, 0x8, wait_for};
        std::array<short, 5> offsets{0, 4, 8, 16, 24};
        for (size_t i = 0; i < buffer.size(); ++i) {
            for (short ii = 0; ii < sizeof(int); ++ii)
                (data.get() + offsets[i])[ii] = reinterpret_cast<const std::byte*>(&buffer[i])[ii];
        }
    int index = 0, len = 61;
    std::apply([&](auto const&... param) {
        (..., (void)([&]()
        {
            LOG(std::format("Index: {0}", index));
            if constexpr (std::is_same_v<std::decay_t<decltype(param)>, const char*>) {
                LOG(std::format("String: {0}; Size: {1}", param, std::strlen(param)));
                size_t new_size = len + 2 + std::strlen(param) + sizeof(int);
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(new_size);
                std::ranges::copy(data.get(), data.get() + len, this_data.get());
                this_data[len] = static_cast<std::byte>(index);
                this_data[len + 1] = std::byte{0x2};
                std::ranges::copy(std::bit_cast<std::array<std::byte, sizeof(int)>>(static_cast<int>(std::strlen(param))), this_data.get() + len + 2);
                std::ranges::copy(std::as_bytes(std::span(param, static_cast<int>(std::strlen(param)))), this_data.get() + len + 6);
                len = new_size;
                data = std::move(this_data);
            }
            else if constexpr (std::is_signed_v<std::decay_t<decltype(param)>> or std::is_unsigned_v<std::decay_t<decltype(param)>>) {
                LOG(std::format("Integer: {0}; Type: {1}", param, (std::is_signed_v<std::decay_t<decltype(param)>>) ? "signed" : "unsigned"));
                size_t new_size = len + 2 + sizeof(int);
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(new_size);
                std::ranges::copy(data.get(), data.get() + len, this_data.get());
                this_data[len] = static_cast<std::byte>(index);
                this_data[len + 1] = std::byte{(std::is_signed_v<std::decay_t<decltype(param)>>) ? 0x9 : 0x5};
                auto i_data = std::as_bytes(std::span(&param, 1));
                std::ranges::copy(const_cast<std::byte*>(i_data.data()), const_cast<std::byte*>(i_data.data()) + i_data.size(), this_data.get() + len + 2);

                len = new_size;
                data = std::move(this_data);
            }
            ++index;
		    if (len >= 61) 
                data[60] = static_cast<std::byte>(index);
        }()
    ));
    }, std::make_tuple(params...));
        ENetPacket* packet = enet_packet_create(data.get(), len, ENET_PACKET_FLAG_RELIABLE);
            if (packet not_eq NULL)
	            if (enet_peer_send(event.peer, 0, packet) == 0) 
                     LOG(std::format("Released Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), len));
                else LOG(std::format("Invalid Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), len) );
    
};
