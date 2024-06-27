#include <ranges> /* std::ranges:: */
#include <algorithm> /* string manipulation */
#include <vector>

#include "include\database\items.hpp"
#include "include\network\enet.hpp"
#include <memory> /* std::unique_ptr<>  */
#include <chrono>
using namespace std::chrono; /* for faster writing. I hate typing std::chrono:: T_T */
#include "include\database\peer.hpp"
#include "include\network\packet.hpp"
#include "include\database\sqlite3.hpp"
#include "include\database\world.hpp"
#include "include\tools\string_view.hpp"
#include "include\tools\random_engine.hpp"

#include "include\action\actions"

int main() 
{
    void github_sync(const char* commit); // -> import github.o
    github_sync("7de0ee666dc27eab6478439a5f3b7adab4a44eb0");
    enet_initialize();
    {
        ENetAddress address{.host = ENET_HOST_ANY, .port = 17091};

        int enet_host_compress_with_range_coder(ENetHost* host); // -> import compress.o
        server = enet_host_create(&address, ENET_PROTOCOL_MAXIMUM_PEER_ID, 1, 0, 0);
            server->checksum = enet_crc32;
            enet_host_compress_with_range_coder(server);
    } /* deletes address */
    {
        struct _iobuf* file;
        if (fopen_s(&file, "items.dat", "rb") == 0) 
        {
            fseek(file, 0, SEEK_END);
            im_data.resize(ftell(file) + 60);
            for (int i = 0; i < 5; ++i)
                *reinterpret_cast<int*>(im_data.data() + i * sizeof(int)) = std::array<int, 5>{0x4, 0x10, -1, 0x0, 0x8}[i];
            *reinterpret_cast<int*>(im_data.data() + 56) = ftell(file);
            long end_size = ftell(file); 
            fseek(file, 0, SEEK_SET);
            fread(im_data.data() + 60, 1, end_size, file);
            std::span<const unsigned char> span = std::span<const unsigned char>(reinterpret_cast<const unsigned char*>(im_data.data()), im_data.size());
                hash = std::accumulate(span.begin(), span.end(), 0x55555555u, 
                    [](auto start, auto end) { return (start >> 27) + (start << 5) + end; });
        } /* deletes span, deletes end_size */
        fclose(file);
    }
    cache_items();

    ENetEvent event{};
    while(true)
        while (enet_host_service(server, &event, 1) > 0) /* waits 1 millisecond. it's a good pratice in C++ to always have a small timer for loops cause C++ is so damn fast */
            switch (event.type) 
            {
                case ENET_EVENT_TYPE_CONNECT: /* TODO: investigate random window freezing, could be Growtopia client or enet_peer_send() byte overflow */
                    if (enet_peer_send(event.peer, 0, enet_packet_create((const enet_uint8[4]){0x1}, 4, ENET_PACKET_FLAG_RELIABLE)) < 0) break;
                    event.peer->data = new peer{};
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    delete getpeer;
                    break;
                case ENET_EVENT_TYPE_RECEIVE: 
                {
                    std::span packet{reinterpret_cast<char*>(event.packet->data), event.packet->dataLength};
                        std::string header{packet.begin() + 4, packet.end() - 1};
                    switch (std::span{event.packet->data, event.packet->dataLength}[0]) 
                    {
                        case 2: case 3: 
                        {
                            if (header.starts_with("requestedName|") or header.starts_with("tankIDName|"))
                                logging_in(event, header);
                            else if (header.contains("action|refresh_item_data")) 
                                enet_peer_send(event.peer, 0, enet_packet_create(im_data.data(), im_data.size(), ENET_PACKET_FLAG_NO_ALLOCATE));
                            else if (header.contains("action|enter_game"))
                                enter_game(event, header);
                            else if (header.starts_with("action|input\n") and duration_cast<milliseconds>(steady_clock::now() - getpeer->rate_limit[1]) > 400ms)
                                input(event, header);

                            if (header.contains("action|quit_to_exit"))
                                quit_to_exit(event, header);
                            else if (header.contains("action|quit"))
                                enet_peer_disconnect(event.peer, ENET_NORMAL_DISCONNECTION);
                            else if (header.starts_with("action|join_request\n") and duration_cast<seconds>(steady_clock::now() - getpeer->rate_limit[2]) > 1s)
                                join_request(event, header);
                            else gt_packet(*event.peer, 0, false, "OnFailedToEnterWorld"); /* maintain OnRequestWorldSelectMenu, without this it bugs out */
                            break;
                        }
                        case 4: 
                        {
                            std::unique_ptr<state> state{};
                            {
                                std::vector<std::byte> packet(event.packet->dataLength - 4, std::byte{0x00});
                                if ((packet.size() + 4) >= 60)
                                    for (size_t i = 0; i < packet.size(); ++i)
                                        packet[i] = (reinterpret_cast<std::byte*>(event.packet->data) + 4)[i];
                                if (std::to_integer<unsigned char>(packet[12]) bitand 0x8 and packet.size() < static_cast<size_t>(*reinterpret_cast<int*>(&packet[52])) + 56) break;
                                state = get_state(packet);
                            } /* deletes packet ahead of time */
                            switch (state->peer_state) 
                            {
                                case 128 + 0x10: case 128: break; // peer is jumping
                            }
                            switch (state->type) 
                            {
                                case 0: {
                                    if (getpeer->post_enter.try_lock()) 
                                    {
                                        gt_packet(*event.peer, 0, true, "OnSetPos", floats{getpeer->pos[0], getpeer->pos[1]});
                                        gt_packet(*event.peer, 0, true, "OnChangeSkin", -1429995521);
                                    }
                                    getpeer->pos = state->pos;
                                    getpeer->facing_left = state->peer_state bitand 0x10;
                                    state_visuals(event, *state);
                                    break;
                                }
                                case 3: 
                                {
                                    if (duration_cast<milliseconds>(steady_clock::now() - getpeer->rate_limit[0]) <= 200ms) break;
                                    getpeer->rate_limit[0] = steady_clock::now();
                                    short block1D = state->punch[1] * 100 + state->punch[0]; /* 2D (x, y) to 1D ((destY * y + destX)) formula */
                                    if (state->id == 18) /* punching blocks */
                                    {
                                        // ... TODO add a timer that resets hits every 6-8 seconds (threaded stopwatch)
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].bg == 0 and worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 0) break;
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 8 or worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 6) {
                                            gt_packet(*event.peer, 0, false, "OnTalkBubble", getpeer->netid, worlds[getpeer->recent_worlds.back()].blocks[block1D].fg == 8 ? 
                                                "It's too strong to break." : "(stand over and punch to use)");
                                            break;
                                        }
                                        block_punched(event, *state, block1D);
                                        auto w = std::make_unique<world>(worlds[getpeer->recent_worlds.back()]);
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].fg not_eq 0)
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[0] < items[worlds[getpeer->recent_worlds.back()].blocks[block1D].fg].hits) break;
                                        else /* block broke */
                                        {
                                            worlds[getpeer->recent_worlds.back()].blocks[block1D].fg = 0; 
                                            overwrite_tile(w, block1D, worlds[getpeer->recent_worlds.back()].blocks[block1D]); /* update world.db for breaking block */
                                            state_visuals(event, *state);
                                        }
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].bg not_eq 0)
                                        if (worlds[getpeer->recent_worlds.back()].blocks[block1D].hits[1] < items[worlds[getpeer->recent_worlds.back()].blocks[block1D].bg].hits) break;
                                        else /* block broke */
                                        {
                                            worlds[getpeer->recent_worlds.back()].blocks[block1D].bg = 0;
                                            overwrite_tile(w, block1D, worlds[getpeer->recent_worlds.back()].blocks[block1D]); /* update world.db for breaking block */
                                            state_visuals(event, *state);
                                        }
                                    }
                                    else /* placing blocks */
                                    {
                                        state_visuals(event, *state);
                                    }
                                    break;
                                }
                                default:
                                    state_visuals(event, *state); /* other visuals that will be sent to everyone in world */
                                    break;
                            }
                            break;
                        }
                    }
                    enet_packet_destroy(event.packet); /* cleanup */
                    break;
                }
            }
    return 0;
}
