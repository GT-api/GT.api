
namespace engine
{
    class simple
    {
        unsigned seed;
        static constexpr unsigned a = 1664525;
        static constexpr unsigned c = 1013904223;
    public:
        explicit simple(unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())) 
            : seed(seed) {}

        unsigned uint32(const std::array<unsigned, 2>& range) 
        { 
            seed = a * seed + c;
            return range[0] + (seed % (range[1] - range[0] + 1)); 
        }

        float float32(const std::array<float, 2>& range) 
        {
            seed = a * seed + c;
            return range[0] + static_cast<float>(seed) / static_cast<float>(std::numeric_limits<unsigned>::max()) * (range[1] - range[0]);
        }
    };
}