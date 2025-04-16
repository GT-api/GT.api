#include "items.hpp"
#include "peer.hpp"
#include "network/packet.hpp"
#include "world.hpp"

std::unordered_map<std::string, world> worlds;

void send_data(ENetPeer& peer, const std::vector<std::byte>& data)
{
    std::size_t size = data.size();
    if (size < 14) return;
    auto packet = enet_packet_create(nullptr, size + 5, ENET_PACKET_FLAG_RELIABLE);
    if (packet == nullptr or packet->dataLength < (size + 4)) return;
    *reinterpret_cast<std::array<enet_uint8, 4>*>(packet->data) = { 0x4 };
    memcpy(packet->data + 4, data.data(), size); // @note for safety reasons I will not reinterpret the values.
    if (size >= 13 + sizeof(std::size_t)) 
    {
        std::size_t resize_forecast = *std::bit_cast<std::size_t*>(data.data() + 13); // @note we just wanna see if we can resize safely
        if (static_cast<int>(data[12]) bitand 0x8) // @note data[12] = peer_state in state class.
        {
            if (resize_forecast <= 512 and packet->dataLength + resize_forecast <= 512)
                enet_packet_resize(packet, packet->dataLength + resize_forecast);
        }
    }
    enet_peer_send(&peer, 1, packet);
}

void state_visuals(ENetEvent& event, state s) 
{
    s.netid = _peer[event.peer]->netid;
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
            send_data(p, compress_state(s));
    });
}

void block_punched(ENetEvent& event, state s, const int block1D) 
{
    worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D].fg == 0 ?
    worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D].hits[1]++ :
    worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D].hits[0]++;
    s.type = 8; /* change packet type from 3 to 8. */
    s.id = 6; /* hit phase visuals */
	state_visuals(event, s);
}

void drop_visuals(ENetEvent& event, const std::array<short, 2>& im, const std::array<float, 2>& pos) 
{
    std::vector<ifloat>& ifloats{worlds[_peer[event.peer]->recent_worlds.back()].ifloats};
    ifloat it = ifloats.emplace_back(ifloat{ifloats.size() + 1, im[0], im[1], pos}); // @note a iterator ahead of time
    std::vector<std::byte> compress = compress_state({
        .type = 14, 
        .netid = -1, 
        .id = it.id, 
        .pos = {it.pos[0] * 32, it.pos[1] * 32}
    });
    *reinterpret_cast<int*>(&compress[8]) = it.uid;
    *reinterpret_cast<float*>(&compress[16]) = static_cast<float>(it.count);
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (not _peer[&p]->recent_worlds.empty() and not _peer[event.peer]->recent_worlds.empty() and _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
            send_data(p, compress);
    });
}

void clothing_visuals(ENetEvent &event) 
{
    gt_packet(*event.peer, true, {
        "OnSetClothing", 
        std::vector<float>{_peer[event.peer]->clothing[hair], _peer[event.peer]->clothing[shirt], _peer[event.peer]->clothing[legs]}, 
        std::vector<float>{_peer[event.peer]->clothing[feet], _peer[event.peer]->clothing[face], _peer[event.peer]->clothing[hand]}, 
        std::vector<float>{_peer[event.peer]->clothing[back], _peer[event.peer]->clothing[head], _peer[event.peer]->clothing[charm]}, 
        (int)-1429995521,
        std::vector<float>{_peer[event.peer]->clothing[ances], 0.0f, 0.0f}
    });
}
