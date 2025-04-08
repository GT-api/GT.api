constexpr std::array<enet_uint8, 4> connect_data = {0x1};

void type_connect(ENetEvent event) 
{
    enet_peer_send(event.peer, 0, enet_packet_create([] {
        return connect_data.data(); 
    }(), connect_data.size(), ENET_PACKET_FLAG_RELIABLE));
    std::shared_ptr<peer> peer_data = std::make_shared<peer>();
    event.peer->data = peer_data.get();
    _peer.emplace(event.peer, peer_data);
}
