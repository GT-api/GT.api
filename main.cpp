/*
    GT.api (c) 2024
    Project has open arms for contribution (friendly, no stress)
    @author leeendl | Discord: leeendl
*/
#include "include\database\items.hpp" // @note items.dat reading
#include "include\network\enet.hpp" // @note ENet supporting AF_INET6
#include "include\database\sqlite3.hpp" // @note SQLlite library
#include "include\database\peer.hpp" // @note everything relating to the peer
#include "include\network\packet.hpp" // @note back-end packet dealing (using ENet & basic C++ concepts)
#include "include\tools\string_view.hpp" // @note stuff that isn't in the standard C++ library
#include "include\database\world.hpp" // @note everything related to a world
#include "include\tools\random_engine.hpp" // @note optimized random number generator

#include "include\on\on"
#include "include\network\jtpool.hpp"
#include "include\action\actions"
#include "include\state\states"

void git_check(const std::string& commit); // -> import git.o
void basic_https(const std::string& s_ip, u_short s_port, u_short https_port); // -> import https.o
bool ip_is_real(const std::string& ip, const std::string& token); // -> import anti-vpn.o
int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o -- Open Source: https://github.com/lsalzman/enet/blob/master/compress.c

int main() 
{
    jtpool jt_handler{};
    git_check("9bc19452a099a5a0c50e584f892f8f9872f9033e");
    enet_initialize();
    server = enet_host_create({.host = in6addr_any, .port = 17091}, ENET_PROTOCOL_MAXIMUM_PEER_ID, 1, 0, 0);
        server->checksum = enet_crc32;
        enet_host_compress_with_range_coder(server);
    std::thread(&basic_https, "127.0.0.1", server->address.port, 443).detach();
    {
        struct _iobuf* file;
        if (fopen_s(&file, "items.dat", "rb") == 0) 
        {
            fseek(file, 0, SEEK_END);
            im_data.resize(ftell(file) + 60);
            for (int i = 0; i < 5; ++i)
                *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) = std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
            *reinterpret_cast<int*>(im_data.data() + 56) = ftell(file);
            long end_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            fread(im_data.data() + 60, 1, end_size, file);
            std::span span{reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size()};
                hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
        } /* @note deletes span, deletes end_size */
        fclose(file);
    }
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0)
            switch (event.type) 
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    event.peer->data = new peer{}; // @todo have faster memory spent on unallowed peers, currently it's deleting on enet_peer_disconnect_later()
                    inet_ntop(AF_INET6, &(event.peer->address.host), getpeer->ipv6, INET6_ADDRSTRLEN);

                    std::string problem{};
                    if (not ip_is_real(getpeer->ipv6, "7fff5d956e4445e6943055fc17fcd0eb")) // @note only when hosting. this will not include localhost (::ffff:127.0.0.1)
                        problem = std::format("`4Can not make a new account!`` Sorry, but IP {} is not permitted to create NEW Growtopia account at this time. (this can be because there is an open proxy/VPN here or abuse has come from this IP) Please try again from another IP address.", getpeer->ipv6).c_str();
                    else if (peers(ENET_PEER_STATE_CONNECTING).size() > 2) 
                        problem = "`4OOPS:`` Too many people logging in at once. Please press `5CANCEL`` and try again in a few seconds.";
                    else if (enet_peer_send(event.peer, 0, enet_packet_create(
                        []{ std::array<enet_uint8, 4> data = {0x1}; return data.data(); }(), 4, ENET_PACKET_FLAG_RELIABLE)) == 0) break;
                    else [[unlikely]] problem = "`4ERROR:`` try reconnecting to the `wserver``.";
                    packet(*event.peer, std::format("action|log\nmsg|{}", problem).c_str());
                    enet_peer_disconnect_later(event.peer, ENET_NORMAL_DISCONNECTION);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: /* @note if peer closes growtopia.exe */
                    quit(event, "");
                    break;
                case ENET_EVENT_TYPE_RECEIVE: 
                {
                    switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                    {
                        case 2: case 3: 
                        {
                            std::string header{std::span{event.packet->data, event.packet->dataLength}.begin() + 4, std::span{event.packet->data, event.packet->dataLength}.end() - 1};
                            printf("%s \n", header.c_str());
                            std::ranges::replace(header, '\n', '|');
                            std::vector<std::string> pipes = readpipe(header);
                            const std::string action{(pipes[0] == "protocol") ? pipes[0] : pipes[0] + "|" + pipes[1]};
                            if (auto i = action_pool.find(action); i not_eq action_pool.end())
                                jt_handler.enqueue(3, [=] { i->second(event, std::ref(header)); });
                            break;
                        }
                        case 4: 
                        {
                            state state{};
                            {
                                std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x00});
                                if ((packet.size() + 4) >= 60)
                                    for (size_t i = 0; i < packet.size(); ++i)
                                        packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and 
                                    packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) break;
                                state = get_state(packet);
                            } /* @note deletes packet ahead of time */
                            if (auto i = state_pool.find(state.type); i not_eq state_pool.end())
                                jt_handler.enqueue(3, [i, event, state = std::move(state)] mutable { i->second(event, state); });
                            break;
                        }
                    }
                    enet_packet_destroy(event.packet); /* cleanup */ // @todo understand timing with jtpool...
                    break;
                }
            }
    return 0;
}
