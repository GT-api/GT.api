#include "pch.hpp"
#include "items.hpp"
#include "peer.hpp"
#include "network/packet.hpp"
#include "world.hpp"

world& world::read(std::string name)
{
    std::ifstream file(std::format("worlds\\{}.json", name));
    if (file.is_open()) 
    {
        nlohmann::json j;
        file >> j;
        this->name = name;
        this->owner = j.contains("owner") ? j["owner"].get<int>() : 00;
        for (const auto& i : j["bs"]) this->blocks.emplace_back(block{i["f"], i["b"], i.contains("l") ? i["l"].get<std::string>() : ""});
        for (const auto& i : j["fs"]) this->ifloats.emplace_back(ifloat{i["u"], i["i"], i["c"], std::array<float, 2ull>{i["p"][0], i["p"][1]}});
    }
    return *this;
}

world::~world()
{
    if (!this->name.empty())
    {
        nlohmann::json j;
        if (this->owner != 00) j["owner"] = this->owner;
        for (const auto& block : this->blocks)
        {
            nlohmann::json list = {{"f", block.fg}, {"b", block.bg}};
            if (not block.label.empty()) list["l"] = block.label;
            j["bs"].push_back(list);
        }
        for (const auto& ifloat : this->ifloats)
            if (ifloat.id != 0 || ifloat.count != 0) // @todo handle this
                j["fs"].push_back({{"u", ifloat.uid}, {"i", ifloat.id}, {"c", ifloat.count}, {"p", ifloat.pos}});
        
        std::ofstream(std::format("worlds\\{}.json", this->name)) << j;
    }
}


std::unordered_map<std::string, world> worlds;

void send_data(ENetPeer& peer, const std::vector<std::byte>& data)
{
    std::size_t size = data.size();
    if (size < 14) return;
    ENetPacket *packet = enet_packet_create(nullptr, size + 5, ENET_PACKET_FLAG_RELIABLE);
    if (packet == nullptr || packet->dataLength < (size + 4)) return;
    packet->data[0] = { 04 };
    memcpy(packet->data + 4, data.data(), size); // @note for safety reasons I will not reinterpret the values.
    if (size >= 13 + sizeof(std::size_t)) 
    {
        std::size_t resize_forecast = *std::bit_cast<std::size_t*>(data.data() + 13); // @note we just wanna see if we can resize safely
        if (std::to_integer<unsigned char>(data[12]) & 0x8) // @note data[12] = peer_state in state class.
        {
            if (resize_forecast <= 512 && packet->dataLength + resize_forecast <= 512)
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
        if (!_peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && 
            _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
        {
            send_data(p, compress_state(s));
        }
    });
}

void block_punched(ENetEvent& event, state s, block &b)
{
    (b.fg == 0) ? ++b.hits[1] : ++b.hits[0];
    s.type = 8; /* change packet type from 3 to 8. */
    s.id = 6; /* hit phase visuals */
	state_visuals(event, s);
}

void drop_visuals(ENetEvent& event, const std::array<short, 2ull>& im, const std::array<float, 2ull>& pos) 
{
    std::vector<ifloat>& ifloats{worlds[_peer[event.peer]->recent_worlds.back()].ifloats};
    ifloat it = ifloats.emplace_back(ifloat{ifloats.size() + 1, im[0], im[1], pos}); // @note a iterator ahead of time
    std::vector<std::byte> compress = compress_state({
        .type = 0x0e, 
        .netid = -1, 
        .peer_state = 1,
        .id = it.id, 
        .pos = {it.pos[0] * 32, it.pos[1] * 32}
    });
    *reinterpret_cast<int*>(&compress[8]) = it.uid;
    *reinterpret_cast<float*>(&compress[16]) = static_cast<float>(it.count);
    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) 
    {
        if (!_peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() && 
            _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back())
        {
            send_data(p, compress);
        }
    });
}

void clothing_visuals(ENetEvent &event) 
{
    gt_packet(*event.peer, true, 0, {
        "OnSetClothing", 
        std::vector<float>{_peer[event.peer]->clothing[hair], _peer[event.peer]->clothing[shirt], _peer[event.peer]->clothing[legs]}, 
        std::vector<float>{_peer[event.peer]->clothing[feet], _peer[event.peer]->clothing[face], _peer[event.peer]->clothing[hand]}, 
        std::vector<float>{_peer[event.peer]->clothing[back], _peer[event.peer]->clothing[head], _peer[event.peer]->clothing[charm]}, 
        _peer[event.peer]->skin_color,
        std::vector<float>{_peer[event.peer]->clothing[ances], 0.0f, 0.0f}
    });
}

void tile_update(ENetEvent &event, state s, block &b, world& w) 
{
    s.type = 5; // @note PACKET_SEND_TILE_UPDATE_DATA
    s.peer_state = 0x08;
    std::vector<std::byte> data = compress_state(s);

    short pos = 56;
    data.resize(pos + 8); // @note {2} {2} 00 00 00 00
    *reinterpret_cast<short*>(&data[pos]) = b.fg; pos += sizeof(short);
    *reinterpret_cast<short*>(&data[pos]) = b.bg; pos += sizeof(short);
    pos += sizeof(short); // @todo
    pos += sizeof(short); // @todo (water = 00 04)

    switch (items[b.fg].type)
    {
        case std::byte{ type::DOOR }:
        {
            data[pos - 2] = std::byte{ 01 };
            short len{ static_cast<short>(b.label.length()) };
            data.resize(pos + 1 + 2 + len + 1); // @note 01 {2} {} 0 0

            data[pos] = std::byte{ 01 }; pos += sizeof(std::byte);
            
            *reinterpret_cast<short*>(&data[pos]) = len; pos += sizeof(short);
            std::span<const char> label{b.label.data(), b.label.size()};
            for (const char& c : label) data[pos++] = static_cast<std::byte>(c);

            pos += sizeof(std::byte); // @note '\0'
            break;
        }
        case std::byte{ type::SIGN }:
        {
            data[pos - 2] = std::byte{ 0x19 };
            short len{ static_cast<short>(b.label.length()) };
            data.resize(pos + 1 + 2 + len + 4); // @note 02 {2} {} ff ff ff ff

            data[pos] = std::byte{ 02 }; pos += sizeof(std::byte);

            *reinterpret_cast<short*>(&data[pos]) = len; pos += sizeof(short);
            std::span<const char> label{ b.label.data(), b.label.size() };
            for (const char& c : label) data[pos++] = static_cast<std::byte>(c);

            *reinterpret_cast<int*>(&data[pos]) = -1; pos += sizeof(int); // @note ff ff ff ff
            break;
        }
    }

    peers(ENET_PEER_STATE_CONNECTED, [&](ENetPeer& p) {
        if (!_peer[&p]->recent_worlds.empty() && !_peer[event.peer]->recent_worlds.empty() &&
            _peer[&p]->recent_worlds.back() == _peer[event.peer]->recent_worlds.back()) 
        {
            send_data(p, data);
        }
    });
}