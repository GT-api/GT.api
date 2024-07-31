
void quit(ENetEvent event, const std::string& header) 
{
    if (event.peer->data not_eq nullptr) 
    {
        delete static_cast<peer*>(event.peer->data);
        event.peer->data = nullptr;
    }
    if (event.peer not_eq nullptr) enet_peer_reset(event.peer);
}