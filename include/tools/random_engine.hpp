
namespace engine
{
    /* Simple LCP Random Engine API 1.0.2 made by LeeEndl */
    class simple
    {
        unsigned seed;
    public:
        // @param seed advice using time(0) for the optimial outcome.
        simple(unsigned seed) : seed(std::clamp<unsigned>(seed, 0, UINT_MAX)) {}

        unsigned operator()() 
        {
            seed = 1664525 * seed + 1013904223;
            return seed;
        }
    };
}

unsigned scope(engine::simple& seed, unsigned min, unsigned max) 
{
    unsigned range = max - min + 1;
    return min + (seed() % range);
}