
void respawn(ENetEvent event, const std::string& header) 
{
    gt_packet(*event.peer, true, "OnSetFreezeState", 2);
    gt_packet(*event.peer, true, "OnKilled");
    std::this_thread::sleep_for(1.9s);
    gt_packet(*event.peer, true, "OnSetPos", std::vector<float>{getpeer->rest_pos.front(), getpeer->rest_pos.back()});
    gt_packet(*event.peer, true, "OnSetFreezeState");
}