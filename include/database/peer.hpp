#pragma once

#include "network/enet.hpp" // @note ENet supporting AF_INET6

/* id, count */
class slot {
public:
    short id{0};
    short count{0}; // @note total amount of that item
};

#include <mutex> // @note std::once_flag
#include <deque>
#include <array>
#include <unordered_map>

class peer {
public:
    std::once_flag logging_in{};
    std::once_flag entered_game{};
    std::once_flag welcome_message{};
    std::mutex post_enter{};

    signed netid{-1}; /* peer's netid is world identity. this will be useful for many packet sending */
    int user_id{}; // @note unqiue user id.
    std::array<const char*, 2> ltoken{}; // @note peer's ltoken e.g. [growid, password]
    std::array<float, 10> clothing{}; // @note peer's clothing

    std::array<float, 2> pos{}; // @note position {x, y}
    std::array<float, 2> rest_pos{}; // @note respawn position {x, y}
    bool facing_left{}; // @note peer is directed towards the left direction

    short slot_size{16}; /* amount of slots this peer has | were talking total slots not itemed slots, to get itemed slots do slot.size() */
    std::vector<slot> slots{{18, 1}, {32, 1}}; /* an array of each slot. storing id, count */
    /*
    * @brief set slot::count to nagative value if you want to remove an amount. 
    * @return the remaining amount if exeeds 200. e.g. emplace(slot{0, 201}) returns 1.
    */
    int emplace(slot s) 
    {
        if (auto it = std::find_if(slots.begin(), slots.end(), [&](const auto& found) { return found.id == s.id; }); it != slots.end()) 
        {
            int excess = std::max(0, (it->count + s.count) - 200);
            it->count = std::min(it->count + s.count, 200);
            return excess;
        }
        else slots.emplace_back(std::move(s)); // @note no such item in inventory, so we create a new entry.
        return 0;
    }

    std::array<std::string, 5> recent_worlds{}; // @note recent worlds, a list of 5 worlds, once it reaches 6 it'll be replaced by the oldest
    bool lobby{true}; // @note checks if peer is in EXIT or 'lobby'
    
    std::array<std::chrono::steady_clock::time_point, 3> rate_limit{}; // @note rate limit objects
    std::deque<std::chrono::steady_clock::time_point> messages; // @note last 5 que messages sent time, this is used to check for spamming
};
extern std::unordered_map<ENetPeer*, std::shared_ptr<peer>> _peer;

/* 
* @param pos please resize peer::rate_limit to fit the pos provided, understand the rules! if pos is 5, then size should be 6. 
* @return false if ratelimited
*/
bool create_rt(ENetEvent& event, std::size_t pos, int64_t length);

#include <bits/std_function.h> // @note std::function<>
extern ENetHost* server;

std::vector<ENetPeer> peers(_ENetPeerState state = ENET_PEER_STATE_CONNECTED, std::function<void(ENetPeer&)> fun = [](ENetPeer& peer){});

class state {
    public:
	int type{};
    int netid{};
    // @todo unknown data
    int peer_state{};
    // @todo unknown data
    int id{}; // @note peer's active hand, so 18 (fist) = punching, 32 (wrench) interacting, ect...
    std::array<float, 2> pos{}; // @note position {x, y}
    std::array<float, 2> speed{}; // @note player movement effect (gravity, speed, ect) {x, y}
    std::array<int, 2> punch{}; // @note punching/placing position {x, y}
};

state get_state(const std::vector<std::byte>& packet);

/* put it back into it's original form */
std::vector<std::byte> compress_state(const state& s);

void inventory_visuals(ENetEvent &event);
