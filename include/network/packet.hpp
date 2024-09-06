#include <cstring> // @note std::strlen() - TODO modernize c-style while respecting typename.

/*
@param p short for peer, the peer who will receive this packet, this can also be used with peers() to send to multiple peers.
@param netid to my knowledge this value should be true if it relates to a peer's state in a world (OnRemove, OnSpawn OnChangeSkin, ect), else false (OnConsoleMessage, OnTalkBubble, ect.). 
@param params list of param that structures your packet. each entry will be than identified as a const char* or signed/unsigned or float/double, 
                respectfully void* entires will not be accepted. e.g. classes, ptr, void
*/
template<typename... T>
void gt_packet(ENetPeer& p, bool netid, T... params) {
	std::vector<std::byte> data(61, std::byte(0x00));
        for (std::size_t i = 0; i < 5 * sizeof(int); ++i) 
            data[std::size_t{(i / sizeof(int)) < 2 ? (i / sizeof(int)) * sizeof(int) : (1 << ((i / sizeof(int)) + 1))} + i % sizeof(int)]
                = reinterpret_cast<const std::byte*>(&std::array<int, 5>{4, 1, netid ? _peer[&p]->netid : -1, 8, 0}[i / sizeof(int)])[i % sizeof(int)];
    std::size_t size = data.size();
    std::byte index;
    std::apply([&](auto const&... param) 
    {
        (..., (void)([&]()
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(param)>, const char*>) 
            {
                std::vector<std::byte> t_data(size + 2 + std::strlen(param) + sizeof(int));
                for (std::size_t i = 0; i < size; ++i)
                    t_data[i] = data[i];
                t_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x00, "the console message" -> 0x1 */
                t_data[size + 1] = std::byte{0x2};
                t_data[size + 2] = static_cast<std::byte>(static_cast<std::uint16_t>(std::strlen(param)) & 0xFF); 
                t_data[size + 3] = static_cast<std::byte>((static_cast<std::uint16_t>(std::strlen(param)) >> 8) & 0xFF);
                t_data[size + 4] = t_data[size + 5] = std::byte{0x00};
                /* outcome should be the hexadecimal of param's array char(s). e.g. "hello" = 'h' -> 0x68 'e' -> 0x65 'l' -> 0x6C 'l' -> 0x6C 'o' -> 0x6F */
                for (size_t/* C type size_t */ i = 0; i < std::strlen(param); ++i) 
                    t_data[size + 6 + i] = static_cast<std::byte>(param[i]); /* e.g. 'a' -> 0x61. 'z' = 0x7A */ // be educated: https://en.cppreference.com/w/cpp/language/ascii
                size = size + 2 + std::strlen(param) + sizeof(int);
                data = std::move(t_data);
            }
            else if constexpr (std::is_signed_v<std::decay_t<decltype(param)>> or std::is_unsigned_v<std::decay_t<decltype(param)>>) 
            {
                std::vector<std::byte> t_data(size + 2 + sizeof(int));
                for (std::size_t i = 0; i < size; ++i)
                    t_data[i] = data[i];
                t_data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 0x00, 43562/-43562 -> 0x1 */
                t_data[size + 1] = std::byte{(std::is_signed_v<std::decay_t<decltype(param)>>) ? 0x9 : 0x5}; 
                /* outcome should be the hexadecimal of param value. e.g. 2147483647 -> 0x7FFFFFFF.  */
                for (std::size_t i = 0; i < sizeof(param); ++i)
                    t_data[size + 2 + i] = reinterpret_cast<std::byte const*>(&param)[i];
                size = size + 2 + sizeof(int);
                data = std::move(t_data); /* e.g. data + 00 01 09 7F FF FF FF */ // -> appended, meaning t_data = data. (cause it already overwritten)
                //                             so if we have 2 packets with same info it would be: data + 00 09 7F FF FF FF 01 09 7F FF FF FF 
                //                             (NOTE: data's 61 bytes are state, NetID, delay, ect, and will always be the first inital bytes in a array)
                // e.g. data's 61 bytes: 04 00 00 00 01 00 00 00 FF FF FF FF 00 00 00 00 08 00 (x42 0s)
            }
            else if constexpr (std::is_same_v<std::decay_t<decltype(param)>, std::vector<float>>) 
            {
                std::vector<std::byte> t_data(size + 2 + (sizeof(float) * param.size()));
                for (std::size_t i = 0; i < size; ++i)
                    t_data[i] = data[i];
                t_data[size] = index;
                t_data[size + 1] = static_cast<std::byte>(0x1 + 0x2 * (param.size() - 1));
                for (std::size_t i = 0; i < param.size(); ++i) 
                    for (std::size_t ii = 0; ii < sizeof(param[i]); ++ii)
                        t_data[size + 2 + sizeof(float) * i + ii] = reinterpret_cast<std::byte const*>(&param[i])[ii];
                size = size + 2 + sizeof(float) * param.size();
                data = std::move(t_data);
            }
            index = static_cast<std::byte>(std::to_integer<int>(index) + 1);
		    if (size >= 61) 
                data[60] = index;
        }()
    ));
    }, std::forward_as_tuple(params...));
    ENetPacket* packet = enet_packet_create(data.data(), size, ENET_PACKET_FLAG_RELIABLE);
    if (packet not_eq nullptr and packet->dataLength > 61) enet_peer_send(&p, 0, packet);
};

void packet(ENetPeer& p, const std::string& str) 
{
    std::vector<std::byte> data(4 + str.length(), std::byte{0x0});
    data[0] = static_cast<std::byte>(0x3);
    for (std::size_t i = 0; i < str.length(); ++i) 
        data[4 + i] = static_cast<std::byte>(str[i]);
    enet_peer_send(&p, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}