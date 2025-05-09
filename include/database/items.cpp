#include "pch.hpp"
#include "items.hpp"

std::unordered_map<unsigned short, item> items;
std::vector<std::byte> im_data(60, std::byte{ 00 });

template<typename T>
void shift_pos(std::vector<std::byte>& data, int& pos, T& value) 
{
    for (std::size_t i = 0; i < sizeof(T); ++i) 
        reinterpret_cast<std::byte*>(&value)[i] = data[pos + i];
    pos += sizeof(T);
}

void cache_items()
{
    int pos{60}, count{};
    short version{};
    shift_pos(im_data, pos, version);
    shift_pos(im_data, pos, count);
    static constexpr std::string_view token{"PBG892FXX982ABC*"};
    for (unsigned short i = 0; i < count; ++i)
    {
        item im{};
        
        shift_pos(im_data, pos, im.id); pos += 2; // @note downsize im.id to 2 bit rather then a 4 bit
        pos += 1;
        shift_pos(im_data, pos, im.cat);
        shift_pos(im_data, pos, im.type);
        pos += 1;

        short len = *(reinterpret_cast<short*>(&im_data[pos]));
        pos += sizeof(short);
        im.raw_name.resize(len);
        for (short i = 0; i < len; ++i) 
            im.raw_name[i] = static_cast<char>(im_data[pos] ^ std::byte(token[(i + im.id) % token.length()])), 
            ++pos;

        len = *(reinterpret_cast<short*>(&im_data[pos]));
        pos += sizeof(short) + len;

        pos += 13;
        shift_pos(im_data, pos, im.collision);
        {
            std::byte raw_hits{};
            shift_pos(im_data, pos, raw_hits);
            im.hits = std::to_integer<short>(raw_hits);
            if (im.hits != 0) im.hits /= 6; // @note unknown reason behind why break hit is muliplied by 6 then having to divide by 6
        }
        shift_pos(im_data, pos, im.hit_reset);
        {
            if (im.type == std::byte {type::CLOTHING}) 
            {
                std::byte cloth_type{};
                shift_pos(im_data, pos, cloth_type);
                im.cloth_type = std::to_integer<unsigned short>(cloth_type);
            }
            else pos += 1; // @note assign nothing
            if (im.type == std::byte{type::AURA}) im.cloth_type = clothing::ances;
        }
        shift_pos(im_data, pos, im.rarity);
        pos += 1;

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

        pos += 20;

        shift_pos(im_data, pos, im.mod);
        shift_pos(im_data, pos, im.mod1);

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
            shift_pos(im_data, pos, im.mod2);
            pos += sizeof(std::array<std::byte, 9>);
        }
        if (version >= 13)
            shift_pos(im_data, pos, im.mod3);
        if (version >= 14)
            shift_pos(im_data, pos, im.mod4);
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
            shift_pos(im_data, pos, im.mod5);
        if (version >= 18)
            shift_pos(im_data, pos, im.mod6);
        if (version >= 19)
            pos += sizeof(std::array<std::byte, 9>);
        if (version == 21)
            shift_pos(im_data, pos, im.mod7);
        
        items.emplace(i, im);
    }
    printf("cached %d items.\n", count);
}
