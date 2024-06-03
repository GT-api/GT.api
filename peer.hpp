/* child: packet.hpp */
#define DEBUG

#if defined(DEBUG)
#define LOG(message) std::clog << (message) << std::endl
#else
#define LOG(message)
#endif

class peer {
public:
    bool LoginLoop{false};
    signed netid{-1}; 
};

#define getpeer static_cast<peer*>(event.peer->data)
