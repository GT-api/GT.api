
bool ip_is_real(const std::string& ip, const std::string& token); // -> import anti-vpn.o

void type_connect(ENetEvent event) 
{
    event.peer->data = new peer{}; // @todo have faster memory spent on unallowed peers, currently it's deleting on enet_peer_disconnect_later()
    inet_ntop(AF_INET6, &(event.peer->address.host), getpeer->ipv6, INET6_ADDRSTRLEN);
    enet_peer_send(event.peer, 0, enet_packet_create(
        []{ std::array<enet_uint8, 4> data = {0x1}; return data.data(); }(), 4, ENET_PACKET_FLAG_RELIABLE));
}