#include "pch.hpp"
#include "database/items.hpp"
#include "database/peer.hpp"
#include "action/actions"
#include "state/states"
#include "type_receive.hpp"

#include "tools/string_view.hpp"

std::vector<std::thread> threads{};

void type_receive(ENetEvent event) 
{
    switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
    {
        case 2: case 3: 
        {
            std::string header{std::span{event.packet->data, event.packet->dataLength}.begin() + 4, std::span{event.packet->data, event.packet->dataLength}.end() - 1};
            printf("%s\n", header.c_str());
            std::ranges::replace(header, '\n', '|');
            std::vector<std::string> pipes = readch(header, '|');
            const std::string action{(pipes[0] == "protocol") ? pipes[0] : pipes[0] + "|" + pipes[1]};
            if (auto i = action_pool.find(action); i not_eq action_pool.end())
                i->second(event, header);
            break;
        }
        case 4: 
        {
            state state{};
            {
                std::vector<std::byte> packet{event.packet->dataLength - 4};
                {
                    std::size_t size = packet.size();
                    if ((size + 4) >= 60)
                        for (std::size_t i = 0; i < size; ++i)
                            packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                    if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and 
                        size < static_cast<std::size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) break;
                } // @note deletes size
                state = get_state(packet);
            } // @note deletes packet
            if (auto i = state_pool.find(state.type); i not_eq state_pool.end())
                i->second(event, std::move(state));
            break;
        }
    }
    enet_packet_destroy(event.packet);
}