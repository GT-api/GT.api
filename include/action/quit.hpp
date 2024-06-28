
void quit(ENetEvent& event, std::string& header) 
{
    enet_peer_disconnect(event.peer, ENET_NORMAL_DISCONNECTION);
}