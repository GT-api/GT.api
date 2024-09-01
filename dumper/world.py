# world parser, also JSON format

from typing import List
import orjson
import struct
import sys
import math

import common

f = None
f_out = None

world_info = {}

def get_str() -> str:
    return common.get_str(f)

def get_int(sz) -> int: 
    return common.get_int(sz, f)

def get_list(len_sz, elm_sz) -> List:
    return common.get_list(len_sz, elm_sz, f)

def get_byte_arr(len) -> bytes:
    return common.get_byte_arr(len, f)

def get_list_int(len_sz, elm_sz) -> List:
    return common.get_list_int(len_sz, elm_sz, f)

def get_float() -> float:
    return common.get_float(f)

def skip(len):
    common.skip(len, f)

def parse_block(i):
    tile = {}
    tile["debug_block_indx"] = i
    tile["debug_curr_pos"] = f.tell()

    tile["x"] = i % int(world_info["width"])
    tile["y"] = math.floor(i / int(world_info["width"]))

    tile["extra_tile_data_type"] = 0

    tile["fg"] = get_int(2)
    tile["bg"] = get_int(2)
    tile["parent_block_index"] = get_int(2)
    tile["item_flags_low"] = get_int(1)
    tile["item_flags_high"] = get_int(1)

    if tile["item_flags_low"] & 0x02:
        tile["lock_block_index"] = get_int(2)
    
    if tile["item_flags_low"] & 0x01:
        tile["extra_tile_data_type"] = get_int(1)

    if tile["extra_tile_data_type"] != 0:
        data = {}
        if tile["extra_tile_data_type"] == 1:
            data["label"] = get_str()
            data["unk1_8"] = get_int(1)

        elif tile["extra_tile_data_type"] == 2:
            data["label"] = get_str()
            skip(4)

        elif tile["extra_tile_data_type"] == 3:
            data["flag"] = get_int(1)
            data["owner_user_id"] = get_int(4)
            temp = get_list(4, 4)
            data["access_count"] = temp.__len__()

            acc_id = []
            for id in temp:
                acc_id.append(int.from_bytes(id, byteorder="little"))

            data["access_list_user_id"] = acc_id            

            non_world_locks = [202, 204, 206, 4994]

            if not tile["fg"] in non_world_locks:
                data["minimum_level"] = get_int(1)
                data["unk2_arr"] = get_byte_arr(7).hex()

        elif tile["extra_tile_data_type"] == 4:
            data["time_left"] = get_int(4)
            data["fruit_count"] = get_int(1)
        
        elif tile["extra_tile_data_type"] == 8:
            data["symbol"] = get_int(1)

        elif tile["extra_tile_data_type"] == 9:
            data["time_left"] = get_int(4) 
            if tile["fg"] == 10656:
                skip(4)

        elif tile["extra_tile_data_type"] == 10:
            data["unk_32"] = get_int(4)
            data["achievement_id"] = get_int(1)

        elif tile["extra_tile_data_type"] == 11:
            data["user_id"] = get_int(4)
            data["growID"] = get_str() 

        elif tile["extra_tile_data_type"] == 14:
            data["label"] = get_str()
            data["unk_8"] = get_int(1)
            data["unk_16"] = get_int(2)
            data["unk_16"] = get_int(2)
            data["hat"] = get_int(2)
            data["shirt"] = get_int(2)
            data["pants"] = get_int(2)
            data["boots"] = get_int(2)
            data["face"] = get_int(2) 
            data["hand"] = get_int(2)
            data["back"] = get_int(2)
            data["hair"] = get_int(2)
            data["neck"] = get_int(2)

        elif tile["extra_tile_data_type"] == 15:
            data["egg_amount"] = get_int(4)  

        elif tile["extra_tile_data_type"] == 16:
            data["team_id"] = get_int(1)

        elif tile["extra_tile_data_type"] == 17:
            pass

        elif tile["extra_tile_data_type"] == 18:
            data["unk_arr"] = get_byte_arr(5).hex()
        
        elif tile["extra_tile_data_type"] == 19:
            data["hat"] = get_int(2)
            data["shirt"] = get_int(2)
            data["pants"] = get_int(2)
            data["shoes"] = get_int(2)
            data["face"] = get_int(2)
            data["hand"] = get_int(2)
            data["back"] = get_int(2)
            data["hair"] = get_int(2)
            data["neck"] = get_int(2)   

        elif tile["extra_tile_data_type"] == 20:
            data["crystal_list"] = get_list(2, 1)

        elif tile["extra_tile_data_type"] == 21:
            data["crime_name"] = get_str()
            data["crime_index"] = get_int(4)
            data["unk_8"] = get_int(1)

        elif tile["extra_tile_data_type"] == 22:
            pass
        
        elif tile["extra_tile_data_type"] == 23:
            data["item_id"] = get_int(4)

        elif tile["extra_tile_data_type"] == 24:
            data["item_id"] = get_int(4)
            data["price"] = get_int(4)

        elif tile["extra_tile_data_type"] == 25:
            data["flags"] = get_int(1)
            data["fishes"] = []
            for i in range(int(get_int(4) / 2)):
                fish_info = {}
                fish_info["item_id"] = get_int(4)
                fish_info["lbs"] = get_int(4)
                data["fishes"].append(fish_info)

        elif tile["extra_tile_data_type"] == 26:
            data["Unk1_40"] = get_byte_arr(5).hex()

        elif tile["extra_tile_data_type"] == 27:
            data["temperature"] = get_int(4)

        elif tile["extra_tile_data_type"] == 28:
            data["harvested"] = get_int(1)
            data["unk1_16"] = get_int(2)
            data["unk2_16"] = get_int(2)
            data["decoration_percentage"] = get_int(1) 

        elif tile["extra_tile_data_type"] == 30:
            data["instrument_type"] = get_int(1)
            data["note"] = get_int(4)

        elif tile["extra_tile_data_type"] == 31:
            data["flags"] = get_int(1)
            data["name"] = get_str()
            data["age_sec"] = get_int(4)
            data["unk1_32"] = get_int(4)
            data["unk2_32"] = get_int(4) 
            data["can_be_fed"] = get_int(1)
            data["food_saturation"] = get_int(4)
            data["water_saturation"] = get_int(4)
            data["color_argb"] = get_byte_arr(4).hex()
            data["sick_duration"] = get_int(4)

        elif tile["extra_tile_data_type"] == 32:
            data["bolt_list_id"] = get_list_int(4, 4)

        elif tile["extra_tile_data_type"] == 33:
            if tile["fg"] == 3394: 
                data["country"] = get_str()
            pass

        elif tile["extra_tile_data_type"] == 34:
            data = [] 

        elif tile["extra_tile_data_type"] == 35:
            data["item_id"] = get_int(4)
            data["label"] = get_str()

        elif tile["extra_tile_data_type"] == 36:
            data["label"] = get_str()
            data["base_pet"] = get_int(4)
            data["combined_pet_1"] = get_int(4)
            data["combined_pet_2"] = get_int(4)

        elif tile["extra_tile_data_type"] == 37:
            data["name"] = get_str()
            data["pet_total_count"] = get_int(4)
            data["unk_32"] = get_int(4)
            data["pets"] = []
            for i in range(int(data["pet_total_count"])):
                data["pets"].append(get_int(4))

        elif tile["extra_tile_data_type"] == 38:
            data["temperature"] = get_int(4)

        elif tile["extra_tile_data_type"] == 39:
            data["time_passed_sec"] = get_int(4)

        elif tile["extra_tile_data_type"] == 40:
            data["settings"] = get_byte_arr(4).hex()

        elif tile["extra_tile_data_type"] == 41:
            data["ghost_jar_count"] = get_int(1)

        elif tile["extra_tile_data_type"] == 42:
            data["prize_id"] = get_int(4)

        elif tile["extra_tile_data_type"] == 43:
            data["unk_arr"] = get_byte_arr(5).hex()

        elif tile["extra_tile_data_type"] == 44:
            data["fish_type_id"] = get_int(4)
            data["size"] = get_int(4)
        
        elif tile["extra_tile_data_type"] == 45:
            data["item_id"] = get_int(4)
            data["unk_8"] = get_int(1)

        elif tile["extra_tile_data_type"] == 46:
            data["enemy_id"] = get_int(4)
            data["prize_id"] = get_int(4)
            data["flag"] = get_int(4)

        tile["extra_data"] = data

    if tile["item_flags_high"] & 8:
        get_int(4)
        skip(16)

    return tile


def parse_world(path_in, path_out):
    global f, f_out
    f = open(path_in, "rb")
    f_out = open(path_out, "wb")

    world_info["width"] = get_int(4)
    world_info["height"] = get_int(4)
    world_info["count"] = world_info["width"] * world_info["height"]
    world_info["world_name"] = get_str()
    
    if world_info["width"] < 1 or world_info["height"] < 1:
        print("error file size")
        sys.exit(0)

    if world_info["count"] > 32768:
        print("skipped", world_info["count"] - 32768)
        world_info["count"] = 32768

    print(world_info)
    f_out.write(orjson.dumps(world_info) + b"\n")

    tile_list = []
    for i in range(world_info["count"]):
        tile = parse_block(i)
        tile_list.append(tile)

    f_out.write(orjson.dumps(tile_list) + b"\n")
    
    f_out.close()
    f.close()

parse_world("/mnt/data/row_data.bin", "/mnt/data/parsed_world_data.json")
