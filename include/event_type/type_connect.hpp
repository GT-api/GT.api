
void type_connect(ENetEvent event) 
{
    enet_peer_send(event.peer, 0, enet_packet_create(
        []{ std::array<enet_uint8, 4> data = {0x1}; return data.data(); }(), 4, ENET_PACKET_FLAG_RELIABLE));
    std::shared_ptr<peer> peer_data = std::make_shared<peer>();
    event.peer->data = peer_data.get();
    _peer.emplace(event.peer, peer_data);
}
