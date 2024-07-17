
void quit(ENetEvent event, const std::string& header) 
{
    enet_peer_reset(event.peer);
}