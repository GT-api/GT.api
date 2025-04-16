#pragma once
#include <cstring> // @note std::strlen() - TODO modernize c-style while respecting typename.
#include <any>

/*
@param p short for peer, the peer who will receive this packet, this can also be used with peers() to send to multiple peers.
@param netid to my knowledge this value should be true if it relates to a peer's state in a world (OnRemove, OnSpawn OnChangeSkin, ect), else false (OnConsoleMessage, OnTalkBubble, ect.). 
@param params list of param that structures your packet. each entry will be than identified as a const char* or signed/unsigned or float/double, 
                respectfully void* entires will not be accepted. e.g. classes, ptr, void
*/
void gt_packet(ENetPeer& p, bool netid, const std::vector<std::any>& params);

void action(ENetPeer& p, const std::string& action, const std::string& str);