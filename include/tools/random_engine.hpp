#include <random> /* std::minstd_rand */
#include <chrono> /* std::chrono:: */

class seed 
{
    std::minstd_rand lcg;
    std::uniform_int_distribution<uint_fast32_t> uniform;
public:
    seed() : lcg(std::chrono::system_clock::now().time_since_epoch().count()) {}
    uint_fast32_t fast(const uint_fast32_t min, const uint_fast32_t max) 
    {
        return (uniform.param(std::uniform_int_distribution<uint_fast32_t>
            ::param_type(min, max > 2147483647UL ? 2147483647UL : max)), uniform(lcg)); 
    }
};
