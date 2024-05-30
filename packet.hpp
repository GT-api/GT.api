#include <enet/packet.c>
#include <array>
#include <numeric>
#include <memory>
#include <vector> 
#include <cstring>
#include <algorithm>

template<typename... T>
void gt_packet(ENetPeer* peer, T... params) {
	std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(61);
        std::fill_n(data.get(), 61, std::byte{0});
        std::array<int, 5> buffer{0x4, 0x1, -1, 0x8, 0};
        std::array<short, 5> offsets{0, 4, 8, 16, 24};
        for (size_t i = 0; i < buffer.size(); ++i) {
            for (short ii = 0; ii < sizeof(int); ++ii)
                (data.get() + offsets[i])[ii] = reinterpret_cast<const std::byte*>(&buffer[i])[ii];
        }
    int index = 0, len = 0;
    std::apply([&](auto const&... param) {
        (..., (void)([&]()
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(param)>, const char*>) {
                size_t new_size = len + 2 + std::strlen(param) + sizeof(int);
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(new_size);
                std::copy_n(data.get(), len, this_data.get());
                this_data[len] = static_cast<std::byte>(index);
                this_data[len + 1] = std::byte{0x2};
                int str_len = static_cast<int>(std::strlen(param));
                std::ranges::copy(std::as_bytes(std::span(&str_len, 1)), this_data.get() + len + 2);
                std::ranges::copy(std::as_bytes(std::span(param, str_len)), this_data.get() + len + 6);

                len = new_size;
                data = std::move(this_data);
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(param)>, int>) {
                size_t new_size = len + 2 + sizeof(int);
                std::unique_ptr<std::byte[]> this_data = std::make_unique<std::byte[]>(new_size);
                memcpy(this_data.get(), data.get(), len);
                this_data[len] = static_cast<std::byte>(index);
                this_data[len + 1] = std::byte{0x9};
                memcpy(this_data.get() + len + 2, &param, sizeof(int));
    
                len = new_size;
                data = std::move(this_data);
            }
            index++;
		    if (len >= 61) 
                data[60] = static_cast<std::byte>(index);
        }()
    ));
    }, std::make_tuple(params...));
    ENetPacket* packet = enet_packet_create(data.get(), len, 1);
    if (packet not_eq NULL)
	   enet_peer_send(peer, 0, packet), LOG(std::format("Released Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), len));
    else LOG(std::format("Invalid Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), len) );
    
};
