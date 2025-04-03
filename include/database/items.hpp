
enum clothing
{ 
    none, shirt, legs, feet, face, hand, back, head, charms,ances
};

#include <string> // @note std::string

class item 
{
    public:
    unsigned short id{}; /* item identity */
    std::string raw_name{}; /* the exact name of the item including uppercases */
    short hits{}; // @todo make it unsigned.
    unsigned short type{};
    unsigned short cloth_type{clothing::none}; /* use clothing:: if you are unsure of the order */
}; 
#include <map>
std::map<int, item> items;

#include <vector>
std::vector<std::byte> im_data(60, std::byte{0x00});

template<typename T>
void shift_pos(std::vector<std::byte>& data, int& pos, T& value) 
{
    for (std::size_t i = 0; i < sizeof(T); ++i) 
        reinterpret_cast<std::byte*>(&value)[i] = data[pos + i];
    pos += sizeof(T);
}

#include <algorithm>
#include <ranges>
#include <chrono>
using namespace std::chrono;

#include "macros.hpp"

void cache_items() 
{
    time_start
    try {
        int pos{60}, count{};
        short version{};
        shift_pos(im_data, pos, version);
        printf("items.dat version: %d\n", version);
        shift_pos(im_data, pos, count);
        for (int i = 0; i < count; ++i) 
        {
            item im{};
            
            shift_pos(im_data, pos, im.id); pos += 2; // @note downsize im.id to 2 bit rather then a 4 bit
            pos += 2;

            shift_pos(im_data, pos, im.type); pos -= 1; // @note upsize im.type to 2 bit rather then a 1 bit
            pos += 1;

            short len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short);
            im.raw_name.resize(len);
            for (short i = 0; i < len; ++i) 
                im.raw_name[i] = static_cast<char>(im_data[pos] ^ std::byte(std::string_view{"PBG892FXX982ABC*"}[(i + im.id) % std::string_view{"PBG892FXX982ABC*"}.length()])), 
                ++pos;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            pos += 14;

            unsigned char raw_hits{};
            shift_pos(im_data, pos, raw_hits);
            im.hits = static_cast<short>(raw_hits);
            if (im.hits not_eq 0) im.hits /= 6; // @note unknown reason behind why break hit is muliplied by 6 then having to divide by 6
            
            pos += sizeof(int);

            unsigned char cloth_type{};
            shift_pos(im_data, pos, cloth_type);
            im.cloth_type = static_cast<unsigned short>(cloth_type);

            pos += 3;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            pos += 8;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            pos += 24;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            len = *(reinterpret_cast<short*>(&im_data[pos]));
            pos += sizeof(short) + len;

            pos += sizeof(std::array<std::byte, 80>);
            if (version < 11) throw std::runtime_error("GT.api does not support items.dat less than version 11.");
            if (version >= 11)
            {
                pos += *(reinterpret_cast<short*>(&im_data[pos]));
                pos += sizeof(short);
            }
            if (version >= 12)
            {
                pos += sizeof(int);
                pos += sizeof(std::array<std::byte, 9>);
            }
            if (version >= 13)
                pos += sizeof(int);
            if (version >= 14)
                pos += sizeof(int);
            if (version >= 15)
            {
                pos += sizeof(std::array<std::byte, 25>);
                pos += *(reinterpret_cast<short*>(&im_data[pos]));
                pos += sizeof(short);
            }
            if (version >= 16)
            {
                pos += *(reinterpret_cast<short*>(&im_data[pos]));
                pos += sizeof(short);
            }
            if (version >= 17)
                pos += sizeof(int);
            if (version >= 18)
                pos += sizeof(int);
            if (version >= 19)
                pos += sizeof(std::array<std::byte, 9>);
            if (version == 21)
                pos += sizeof(short);
            if (version > 21) throw std::runtime_error("GT.api does not support items.dat greater than version 21.");
            if (im.id % 2 == 0) 
            {
                if (std::string lower = im.raw_name | std::ranges::views::transform(
                    [&](char c) { return std::tolower(c); }) bitor std::ranges::to<std::string>(); lower.contains("ancestral"))
                        im.cloth_type = clothing::ances;
            }
            items.emplace(i, im);
        }
    } catch (const std::exception& exc) {
        time_end(std::format("catch error in cache_items() [{}]", exc.what()).c_str())
        return;
    }
    time_end("void cache_items()")
}
