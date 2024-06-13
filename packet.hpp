/* parent: items.hpp, peer.hpp */
#include "include/enet.hpp"

#include <cstring> /* std::strlen() */

/*
@param p short for peer, the peer who will receive this packet, this can also be used with peers() to send to multiple peers.
@param wait_for prep the packet ahead of time and send it within the time provided. (milliseconds) e.g. 1000 = 1 second, 60000 = 1 minute
@param params list of param that structures your packet. each entry will be than identified as a const char* or signed/unsigned or float/double, 
                respectfully void* entires will not be accepted. e.g. classes, ptr, void
*/
template<typename... T>
void gt_packet(ENetPeer& p, signed wait_for, T... params) {
	std::unique_ptr<std::byte[]> data = std::make_unique<std::byte[]>(61);
        std::ranges::fill(std::span{data.get(), 61}, std::byte{0x00});
        std::array<int, 5> buffer{0x4, 0x1, -1, 0x8, wait_for}; // TODO imply peer's netid
        for (size_t i = 0; i < buffer.size() * sizeof(int); ++i) 
            data[size_t{(i / sizeof(int)) < 2 ? (i / sizeof(int)) * sizeof(int) : (1 << ((i / sizeof(int)) + 1))} + i % sizeof(int)]
                = reinterpret_cast<const std::byte*>(&buffer[i / sizeof(int)])[i % sizeof(int)];
    size_t size = 61;
    std::byte index{0x00};
    std::apply([&](auto const&... param) {
        (..., (void)([&]()
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(param)>, const char*>) {
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(size + 2 + std::strlen(param) + sizeof(int));
                for (size_t i = 0; i < size; ++i)
                    this_data[i] = data[i];
                this_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x00, "the console message" -> 0x1 */
                this_data[size + 1] = std::byte{0x2};
                this_data[size + 2] = static_cast<std::byte>(static_cast<std::uint16_t>(std::strlen(param)) & 0xFF); 
                this_data[size + 3] = static_cast<std::byte>((static_cast<std::uint16_t>(std::strlen(param)) >> 8) & 0xFF);
                this_data[size + 4] = this_data[size + 5] = std::byte{0x00};
                /* outcome should be the hexadecimal of param's array char(s). e.g. "hello" = 'h' -> 0x68 'e' -> 0x65 'l' -> 0x6C 'l' -> 0x6C 'o' -> 0x6F */
                for (size_t i = 0; i < std::strlen(param); ++i) 
                    this_data[size + 6 + i] = static_cast<std::byte>(param[i]); /* e.g. 'a' -> 0x61. 'z' = 0x7A */ // be educated: https://en.cppreference.com/w/cpp/language/ascii
                size = size + 2 + std::strlen(param) + sizeof(int);
                data = std::move(this_data);
            }
            else if constexpr (std::is_signed_v<std::decay_t<decltype(param)>> or std::is_unsigned_v<std::decay_t<decltype(param)>>) {
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(size + 2 + sizeof(int));
                for (size_t i = 0; i < size; ++i)
                    this_data[i] = data[i];
                this_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x00, 43562/-43562 -> 0x1 */
                this_data[size + 1] = std::byte{(std::is_signed_v<std::decay_t<decltype(param)>>) ? 0x9 : 0x5}; 
                /* outcome should be the hexadecimal of param value. e.g. 2147483647 -> 0x7FFFFFFF.  */
                for (size_t i = 0; i < sizeof(param); ++i)
                    this_data[size + 2 + i] = reinterpret_cast<std::byte const*>(&param)[i];
                size = size + 2 + sizeof(int);
                data = std::move(this_data); /* e.g. data + 00 01 09 7F FF FF FF */ // -> appended, meaning this_data = data. (cause it already overwritten)
                //                             so if we have 2 packets with same info it would be: data + 00 09 7F FF FF FF 01 09 7F FF FF FF 
                //                             (NOTE: data's 61 bytes are state, NetID, delay, ect, and will always be the first inital bytes in a array)
                // e.g. data's 61 bytes: 04 00 00 00 01 00 00 00 FF FF FF FF 00 00 00 00 08 00 (x42 0s)
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(param)>, std::vector<float>>) {
                auto this_data = std::make_unique_for_overwrite<std::byte[]>(size + 2 + sizeof(float));
                for (size_t i = 0; i < size; ++i)
                    this_data[i] = data[i];
                this_data[size] = index;
                this_data[size + 1] = std::byte{0x1 + 0x2 * (param.size() - 1)};
                for (size_t i = 0; i < param.size(); ++i) {
                    for (size_t ii = 0; ii < sizeof(param[i]); ++ii)
                        this_data[size + 2 + sizeof(float) * i + ii] = reinterpret_cast<std::byte const*>(&param[i])[ii];
                }
                size = size + 2 + sizeof(float) * param.size();
                data = std::move(this_data);
            }
            index = static_cast<std::byte>(std::to_integer<int>(index) + 1);
		    if (size >= 61) 
                data[60] = index;
        }()
    ));
    }, std::make_tuple(params...));
	if (enet_peer_send(&p, 0, enet_packet_create(data.get(), size, ENET_PACKET_FLAG_RELIABLE)) == 0) 
        LOG(std::format("Released Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), size));
    else LOG(std::format("Invalid Packet; Name: {0}; Size: {1} bytes", std::get<0>(std::make_tuple(params...)), size) );
    
};
