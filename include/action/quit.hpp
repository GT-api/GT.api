
void quit(ENetEvent event, const std::string& header) 
{
    if (event.peer->data not_eq nullptr) 
    {
        event.peer->data = nullptr;
        _peer.erase(event.peer); // @todo
    }
    if (event.peer not_eq nullptr) enet_peer_reset(event.peer);
}