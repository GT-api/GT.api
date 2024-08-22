
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
unsigned scope(engine::simple& seed, unsigned min, unsigned max) { return min + (seed() % (max - min + 1)); }
