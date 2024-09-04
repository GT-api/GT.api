
namespace engine
{
    class simple
    {
        unsigned seed;
    public:
        explicit simple(unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count())) 
            : seed(std::clamp(seed, 0u, std::numeric_limits<unsigned>::max())) {}
            
        unsigned uint32(std::array<unsigned, 2> range) 
        { 
            seed = 1664525 * seed + 1013904223;
            return range[0] + (seed % (range[1] - range[0] + 1)); 
        }
        float float32(std::array<float, 2> range) 
        {
            seed = 1664525 * seed + 1013904223;
            return range[0] + static_cast<float>(seed) / static_cast<float>(std::numeric_limits<unsigned>::max()) * (range[1] - range[0]);
        }
        double float64(std::array<double, 2> range) 
        {
            seed = 1664525 * seed + 1013904223;
            return range[0] + static_cast<double>(seed) / static_cast<double>(std::numeric_limits<unsigned>::max()) * (range[1] - range[0]);
        }
    };
}