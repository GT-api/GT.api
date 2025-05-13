#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "packet.hpp"

void gt_packet(ENetPeer& p, bool netid, signed delay, const std::vector<std::any>& params) 
{
    std::vector<std::byte> data(61, std::byte{ 00 });
    data[0] = std::byte{ 04 };
    data[4] = std::byte{ 01 };
    if (not netid) *reinterpret_cast<signed*>(&data[8]) = -1;
    else *reinterpret_cast<signed*>(&data[8]) = _peer[&p]->netid;
    data[16] = std::byte{ 0x08 };
    *reinterpret_cast<signed*>(&data[24]) = delay;
    std::size_t size = data.size();
    std::byte index{};
    for (const auto& param : params) 
    {
        if (param.type() == typeid(const char*)) 
        {
            std::string_view str{ std::any_cast<const char*>(param) };
            data.resize(size + 2 + str.length() + sizeof(int));
            data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 00, "the console message" -> 01 */
            data[size + 1] = std::byte{ 02 };
            data[size + 2] = static_cast<std::byte>(str.length() & 0xff);
            data[size + 3] = static_cast<std::byte>(( str.length() >> 8 ) & 0xff);
            /* outcome should be the hexadecimal of param's array char(s). e.g. "hello" = 'h' -> 0x68 'e' -> 0x65 'l' -> 0x6C 'l' -> 0x6C 'o' -> 0x6F */
            for (size_t i = 0; i < str.length(); ++i)
                data[size + 6 + i] = static_cast<std::byte>(str[i]); /* e.g. 'a' -> 0x61. 'z' = 0x7A */ // be educated: https://en.cppreference.com/w/cpp/language/ascii
            size += 2 + str.length() + sizeof(int);
        }
        else if (param.type() == typeid(int) || param.type() == typeid(unsigned)) 
        {
            bool is_signed = (param.type() == typeid(int));
            int value = is_signed ? std::any_cast<int>(param) : static_cast<int>(std::any_cast<unsigned>(param));
            data.resize(size + 2 + sizeof(int) + 2);
            data[size] = index; /* element counter. e.g. "OnConsoleMessage" -> 00, 43562/-43562 -> 01 */
            data[size + 1] = (is_signed) ? std::byte{ 0x09 } : std::byte{ 05 }; 
            /* outcome should be the hexadecimal of param value. e.g. 2147483647 -> 0x7FFFFFFF.  */
            for (std::size_t i = 0; i < sizeof(value); ++i)
                data[size + 2 + i] = reinterpret_cast<std::byte const*>(&value)[i];
            size += 2 + sizeof(int); 
            /* e.g. data + 00 01 09 7F FF FF FF */ // -> appended, meaning t_data = data. (cause it already overwritten)
            //                             so if we have 2 packets with same info it would be: data + 00 09 7F FF FF FF 01 09 7F FF FF FF 
            //                             (NOTE: data's 61 bytes are state, NetID, delay, ect, and will always be the first inital bytes in a array)
            // e.g. data's 61 bytes: 04 00 00 00 01 00 00 00 FF FF FF FF 00 00 00 00 08 00 (x42 0s)
        }
        else if (param.type() == typeid(std::vector<float>)) 
        {
            const std::vector<float>& vec = std::any_cast<const std::vector<float>&>(param);
            data.resize(size + 2 + (sizeof(float) * vec.size()) + 2);
            data[size] = index;
            data[size + 1] = 
                (vec.size() == 1) ? std::byte{ 01 } :
                (vec.size() == 2) ? std::byte{ 03 } :
                (vec.size() == 3) ? std::byte{ 04 } :
                                    std::byte{ 00 };
            for (std::size_t i = 0; i < vec.size(); ++i)
                for (std::size_t ii = 0; ii < sizeof(vec[i]); ++ii)
                    data[size + 2 + sizeof(float) * i + ii] = reinterpret_cast<std::byte const*>(&vec[i])[ii];
            size += 2 + (sizeof(float) * vec.size());
        } else return; // @note this will never pass unless you include a param that Growtopia does not recognize

        index = static_cast<std::byte>(std::to_integer<int>(index) + 1);
        if (size >= 61)
            data[60] = index;
    }

    ENetPacket* packet = enet_packet_create(data.data(), size, ENET_PACKET_FLAG_RELIABLE);
    if (packet != nullptr && packet->dataLength > 61)
        enet_peer_send(&p, 0, packet);
}

void action(ENetPeer& p, const std::string& action, const std::string& str) 
{
    std::string_view _action = "action|" + action + "\n";
    std::vector<std::byte> data(4 + _action.length() + str.length(), std::byte{ 00 });
    data[0] = std::byte{ 03 };
    for (std::size_t i = 0; i < _action.length(); ++i) 
        data[4 + i] = static_cast<std::byte>(_action[i]);
    for (std::size_t i = 0; i < str.length(); ++i) 
        data[4 + _action.length() + i] = static_cast<std::byte>(str[i]);
    enet_peer_send(&p, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}
