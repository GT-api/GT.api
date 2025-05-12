#include "pch.hpp"
#include "items.hpp"
#include "peer.hpp"

std::unordered_map<ENetPeer*, std::shared_ptr<peer>> _peer;

bool create_rt(ENetEvent& event, std::size_t pos, int64_t length) 
{
    auto& last_time = _peer[event.peer]->rate_limit[pos];
    auto now = std::chrono::steady_clock::now();

    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() <= length)
        return false;

    last_time = now;
    return true;
}

ENetHost* server;

std::vector<ENetPeer> peers(_ENetPeerState state, std::function<void(ENetPeer&)> fun) {
    std::vector<ENetPeer> peers{};
    for (ENetPeer& peer : std::ranges::subrange(server->peers, server->peers + server->peerCount)) 
        if (peer.state == ENET_PEER_STATE_CONNECTED)
            fun(peer), peers.emplace_back(peer);
    return peers;
}

state get_state(const std::vector<std::byte>& packet) 
{
    const int* _4bit = reinterpret_cast<const int*>(packet.data());
    const float* _4bit_f = reinterpret_cast<const float*>(packet.data());
    return state{
        .type = _4bit[0],
        .netid = _4bit[1],
        .peer_state = _4bit[3],
        .id = _4bit[5],
        .pos = {_4bit_f[6], _4bit_f[7]},
        .speed = {_4bit_f[8], _4bit_f[9]},
        .punch = {_4bit[11], _4bit[12]}
    };
}

std::vector<std::byte> compress_state(const state& s) 
{
    std::vector<std::byte> data(56, std::byte{ 00 });
    int* _4bit = reinterpret_cast<int*>(data.data());
    float* _4bit_f = reinterpret_cast<float*>(data.data());
    _4bit[0] = s.type;
    _4bit[1] = s.netid;
    _4bit[3] = s.peer_state;
    _4bit[5] = s.id;
    _4bit_f[6] = s.pos[0];
    _4bit_f[7] = s.pos[1];
    _4bit_f[8] = s.speed[0];
    _4bit_f[9] = s.speed[1];
    _4bit[11] = s.punch[0];
    _4bit[12] = s.punch[1];
    return data;
}

void inventory_visuals(ENetEvent &event)
{
	std::size_t size = _peer[event.peer]->slots.size();
    std::vector<std::byte> data(66 + (size * sizeof(int)) + sizeof(int), std::byte( 00 ));
    data[0] = std::byte{ 04 };
    data[4] = std::byte{ 0x9 };
    *reinterpret_cast<int*>(&data[8]) = _peer[event.peer]->netid;
    data[16] = std::byte{ 0x08 };
    *reinterpret_cast<int*>(&data[58]) = std::byteswap<int>(_peer[event.peer]->slot_size);
    *reinterpret_cast<int*>(&data[62]) = std::byteswap<int>(size);
    int* slot_ptr = reinterpret_cast<int*>(data.data() + 66);
    for (const slot &slot : _peer[event.peer]->slots)
        *slot_ptr++ = (static_cast<int>(slot.id) | (static_cast<int>(slot.count) << 16)) & 0x00FFFFFF;
            
	enet_peer_send(event.peer, 0, enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE));
}
