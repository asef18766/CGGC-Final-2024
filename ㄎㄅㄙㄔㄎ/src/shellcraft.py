from typing import Dict, Tuple

def extract_func_info()->Tuple[Dict[str, int], Dict[str, int]]:
    with open("shellcode.asm", "r") as fp:
        adr_map = {}
        func_sz = {}

        sz = 0
        prev_n = ""
        for l in fp.readlines():
            if l.startswith("0000000"):
                _, l = l.split(" <")
                name, offset = l.split("> (File Offset: ")
                offset = int(offset.removesuffix("):\n")[2:], 16)
                #print(f"{name} {hex(offset)}")
                adr_map[name] = offset
                func_sz[prev_n] = offset - sz
                prev_n = name
                sz = offset
        '''
        print(hex(adr_map["get_flag"]))
        print(hex(adr_map["get_flag"] + func_sz["get_flag"]))
        
        print(hex(adr_map["GetProcAddressWithHashInIAT"]))
        print(hex(adr_map["GetProcAddressWithHashInIAT"] + func_sz["GetProcAddressWithHashInIAT"]))
        '''
        return (adr_map, func_sz)

def loadbin()->bytes:
    with open("readfile_shellcode.exe", "rb") as fp:
        return fp.read()

def main():
    binary = loadbin()
    adr_map, func_sz = extract_func_info()
    get_flag_sc = binary[adr_map["get_flag"]:adr_map["get_flag"] + func_sz["get_flag"]]
    GetProcAddressWithHashInIAT_sc = binary[adr_map["GetProcAddressWithHashInIAT"]:adr_map["GetProcAddressWithHashInIAT"] + func_sz["GetProcAddressWithHashInIAT"]]
    with open("sc", "wb") as fp:
        payload = get_flag_sc + b"\x90" * (adr_map["GetProcAddressWithHashInIAT"] - (adr_map["get_flag"] + func_sz["get_flag"])) + GetProcAddressWithHashInIAT_sc
        print(f"size: {len(payload)}")
        fp.write(payload)


main()