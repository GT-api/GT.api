
#define DEBUG

#if defined(DEBUG)
#define LOG(message) std::clog << (message) << std::endl
#else
#define LOG(message)
#endif

class peer {
public:
    int test = 0;
};

#define getpeer static_cast<peer*>(event.peer->data)
