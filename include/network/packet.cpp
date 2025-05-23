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
            data[size] = index; // @note element counter e.g. "OnConsoleMessage" -> 00, "hello" -> 01
            data[size + 1] = std::byte{ 02 };
            data[size + 2] = static_cast<std::byte>(str.length() & 0xff);
            data[size + 3] = static_cast<std::byte>(( str.length() >> 8 ) & 0xff);
            const std::byte *_1bit = reinterpret_cast<const std::byte*>(str.data());
            for (size_t i = 0; i < str.length(); ++i)
                data[size + 6 + i] = _1bit[i]; // @note e.g. 'a' -> 0x61. 'z' = 0x7A, hex tabel: https://en.cppreference.com/w/cpp/language/ascii
            
            size += 2 + str.length() + sizeof(int);
        }
        else if (param.type() == typeid(int) || param.type() == typeid(unsigned)) 
        {
            bool is_signed = (param.type() == typeid(int));
            auto value = is_signed ? std::any_cast<int>(param) : std::any_cast<unsigned>(param);
            data.resize(size + 2 + sizeof(value) + 2);
            data[size] = index; // @note element counter e.g. "OnSetBux" -> 00, 43562/-43562 -> 01
            data[size + 1] = (is_signed) ? std::byte{ 0x09 } : std::byte{ 05 };
            const std::byte *_1bit = reinterpret_cast<const std::byte*>(&value);
            for (std::size_t i = 0; i < sizeof(value); ++i)
                data[size + 2 + i] = _1bit[i];

            size += 2 + sizeof(value);
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
            const std::byte *_1bit = reinterpret_cast<const std::byte*>(vec.data());
            for (std::size_t i = 0; i < sizeof(float) * vec.size(); ++i)
                data[size + 2 + i] = _1bit[i];

            size += 2 + (sizeof(float) * vec.size());
        }
        else return; // @note this will never pass unless you include a param that Growtopia does not recognize

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
