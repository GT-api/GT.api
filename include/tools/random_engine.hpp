
namespace engine
{
    class simple
    {
        unsigned seed;
    public:
        explicit simple(unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())) 
            : seed(std::clamp(seed, 0u, std::numeric_limits<unsigned>::max())) {}

        unsigned operator()() 
        {
            return seed = 1664525 * seed + 1013904223;
        }
    };
}

float scope(engine::simple& seed, std::array<float, 2> range) { return range[0] + static_cast<float>(seed()) / static_cast<float>(std::numeric_limits<unsigned>::max()) * (range[1] - range[0]); }

unsigned scope(engine::simple& seed, std::array<unsigned, 2> range) { return range[0] + (seed() % (range[1] - range[0] + 1)); }