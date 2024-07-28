#include <random> // @note for integer randomization generator/engine.

class seed 
{
    std::minstd_rand lcg; // @note linear intfast generator
    std::uniform_int_distribution<uint_fast32_t> uniform;
public:
    seed() : lcg(system_clock::now().time_since_epoch().count()) {}
    // @brief instant number generator
    // @return uint_fast32_t (unsigned int)
    inline uint_fast32_t fast(const uint_fast32_t min, const uint_fast32_t max) 
    {
        return (uniform.param(std::uniform_int_distribution<uint_fast32_t>
            ::param_type(min, max > 2147483647UL ? 2147483647UL : max)), uniform(lcg)); 
    }
};
