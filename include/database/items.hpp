#pragma once
#include <string> // @note std::string

enum clothing : std::size_t {
    hair, shirt, legs, 
    feet, face, hand, 
    back, head, charm, 
    ances, none
};

class item 
{
    public:
    unsigned short id{}; /* item identity */
    std::string raw_name{}; /* the exact name of the item including uppercases */
    short hits{}; // @todo make it unsigned.
    unsigned short type{};
    std::size_t cloth_type{clothing::none}; /* use clothing:: if you are unsure of the order */
}; 
#include <map>
extern std::map<int, item> items;

#include <vector>
extern std::vector<std::byte> im_data;

template<typename T>
void shift_pos(std::vector<std::byte>& data, int& pos, T& value);

#include <algorithm>
#include <ranges>
#include <chrono>
using namespace std::chrono;

#include "macros.hpp"

void cache_items();