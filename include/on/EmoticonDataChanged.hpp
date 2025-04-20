#pragma once

extern std::unordered_map<std::string_view, std::string_view> emoticon;

void EmoticonDataChanged(ENetEvent& event);
