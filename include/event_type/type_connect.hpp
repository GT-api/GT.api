
inline void type_connect(ENetEvent event) 
{
    constexpr std::array<enet_uint8, 4ull> connect_data = { 01 };
    enet_peer_send(event.peer, 0, enet_packet_create(connect_data.data(), connect_data.size(), ENET_PACKET_FLAG_RELIABLE));
    std::shared_ptr<peer> peer_data = std::make_shared<peer>();
    event.peer->data = peer_data.get();
    _peer.emplace(event.peer, peer_data);
}
