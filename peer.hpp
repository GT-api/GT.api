/* child: packet.hpp */
#define DEBUG

#include <mutex>

#if defined(DEBUG)
#define LOG(message) std::clog << (message) << std::endl
#else
#define LOG(message)
#endif

class peer {
public:
    std::once_flag logging_in{}; /* without this, GT will keep pushing peer into the server. */
    signed netid{-1}; /* peer's netid is world & server identity. this will be useful for many packet sending */
};

#define getpeer static_cast<peer*>(event.peer->data)
