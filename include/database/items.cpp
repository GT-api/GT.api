#include "items.hpp"

std::map<int, item> items; // Define the variable
std::vector<std::byte> im_data(60, std::byte{0x00}); // Define the variable

template<typename T>
void shift_pos(std::vector<std::byte>& data, int& pos, T& value) 
{
    for (std::size_t i = 0; i < sizeof(T); ++i) 
        reinterpret_cast<std::byte*>(&value)[i] = data[pos + i];
    pos += sizeof(T);
}

#include <iostream>

void cache_items() 
{
    TIME_START;
    int pos{60}, count{};
    short version{};
    shift_pos(im_data, pos, version);
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
        {
            unsigned char raw_hits{};
            shift_pos(im_data, pos, raw_hits);
            im.hits = static_cast<short>(raw_hits);
            if (im.hits not_eq 0) im.hits /= 6; // @note unknown reason behind why break hit is muliplied by 6 then having to divide by 6
        }
        pos += sizeof(int);
        {
            unsigned char cloth_type{};
            shift_pos(im_data, pos, cloth_type);
            im.cloth_type = static_cast<std::size_t>(cloth_type);
        }
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
        if (im.id % 2 == 0) 
        {
            if (std::string lower = im.raw_name | std::ranges::views::transform(
                [&](char c) { return std::tolower(c); }) bitor std::ranges::to<std::string>(); lower.contains("ancestral"))
                    im.cloth_type = clothing::ances;
        }
        items.emplace(i, im);
    }
    TIME_END("void cache_items()");
}
