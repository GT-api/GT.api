#include <vector>
#include <ranges>

/* 
@param str the whole content
@brief reads a string and seperates the pipes '|' and stores those pieces of info into a vector.
@return std::vector<std::string> of all the seperations of the pipes '|' e.g. Name|myName -> readpipe[1] = myName
*/
std::vector<std::string> readpipe(const std::string& str) 
{
    std::vector<std::string> separations;
    for (auto&& part : str | std::views::split('|'))
        separations.emplace_back(std::string(std::ranges::begin(part), std::ranges::end(part)));
    return separations;
}

/* @return true if string contains ONLY alpha [a, b, c] or digits [1, 2, 3] */
bool alpha(const std::string& str) 
{
    for (char c : str)
        if (not std::isalnum(static_cast<unsigned char>(c)))
            return false;
    return true;
}