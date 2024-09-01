from typing import List
import struct

FILE_BEG_FILE = 0
FILE_CUR_POS = 1
FILE_END_POS = 2


def get_int(sz, f) -> int: 
    int_arr = f.read(sz)

    return int.from_bytes(int_arr, byteorder="little")
    

def get_list(len_sz, elm_sz, f) -> List:
    list_length = int.from_bytes(f.read(len_sz), byteorder="little")
    res_list: List = []

    for i in range(list_length):
        res_list.append(f.read(elm_sz))

    return res_list

def get_byte_arr(len, f) -> bytes:

    ret = []
    for i in range(len):
        ret.append(int.from_bytes(f.read(1)))

    return bytes(ret)

def get_list_int(len_sz, elm_sz, f) -> List:
    ls = get_list(len_sz, elm_sz, f)
    res_ls = []

    for arr in ls:
        res_ls.append(int.from_bytes(arr, byteorder="little"))

    return res_ls


def get_float(f) -> float:
    return struct.unpack('f', get_byte_arr(4, f))[0]

def skip(len, f):
    f.seek(len, FILE_CUR_POS)

def get_str(f) -> str:

    str_len = get_int(2, f)

    if str_len == 0:
        return ""

    res_str = get_byte_arr(str_len, f).decode("utf-8")
    
    return "".join(res_str)

def get_hex_arr(len, f) -> List:
    ret = []

    for b in get_byte_arr(len, f):
        ret.append(format(b, "02x").upper())

    return ret

def json_default_func(obj):
    if isinstance(obj, bytes):
        return list(obj)

    raise TypeError 
