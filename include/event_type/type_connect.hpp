
bool ip_is_real(const std::string& ip, const std::string& token); // -> import anti-vpn.o

void type_connect(ENetEvent event) 
{
    event.peer->data = new peer{}; // @todo have faster memory spent on unallowed peers, currently it's deleting on enet_peer_disconnect_later()
    inet_ntop(AF_INET6, &(event.peer->address.host), getpeer->ipv6, INET6_ADDRSTRLEN);

    const std::string problem = (not ip_is_real(getpeer->ipv6, "7fff5d956e4445e6943055fc17fcd0eb")) 
            ? std::format("`4Can not make a new account!`` Sorry, but IP {} is not permitted to create NEW Growtopia account at this time. (this can be because there is an open proxy/VPN here or abuse has come from this IP) Please try again from another IP address.", getpeer->ipv6).c_str()
        : (peers(ENET_PEER_STATE_CONNECTING).size() > 2) 
            ? "`4OOPS:`` Too many people logging in at once. Please press `5CANCEL`` and try again in a few seconds." : "";
    if (not problem.empty())
        packet(*event.peer, std::format("action|log\nmsg|{}", problem).c_str()), enet_peer_disconnect_later(event.peer, disconnection::normal);
    else enet_peer_send(event.peer, 0, enet_packet_create(
        []{ std::array<enet_uint8, 4> data = {0x1}; return data.data(); }(), 4, ENET_PACKET_FLAG_RELIABLE));
}