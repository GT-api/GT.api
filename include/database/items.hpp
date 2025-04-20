#pragma once

enum clothing : unsigned short {
    hair, shirt, legs, 
    feet, face, hand, 
    back, head, charm, 
    ances, none
};

#include <string>
#include <cstddef>

class item 
{
    public:
    unsigned short id{}; /* item identity */
    std::string raw_name{}; /* the exact name of the item including uppercases */
    short hits{}; // @todo make it unsigned.
    std::byte cat{};
    std::byte type{};
    unsigned short cloth_type{clothing::none}; /* use clothing:: if you are unsure of the order */
}; 
#include <map>
extern std::map<unsigned short, item> items;

#include <vector>
extern std::vector<std::byte> im_data;

template<typename T>
void shift_pos(std::vector<std::byte>& data, int& pos, T& value);

void cache_items();