
void refresh_item_data(ENetEvent& event, const std::string& header) 
{
    if (getpeer->recent_worlds.back().empty())
        enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_NO_ALLOCATE));
}