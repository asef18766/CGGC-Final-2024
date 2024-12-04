from scapy.utils import rdpcap
from scapy.layers.inet import ICMP, IP
from Crypto.Cipher import ChaCha20
from typing import Tuple

key = b""
nonce = b""

def encryt(data:bytes)->bytes:
    return ChaCha20.new(key=key, nonce=nonce).encrypt(data)

def decryt(data:bytes)->bytes:
    return ChaCha20.new(key=key, nonce=nonce).decrypt(data)

def split_user_data(data:bytes)->Tuple[int, bytes]:
    cmd = data[0]
    data_sz = int.from_bytes(data[1:5], 'little')
    return (cmd, data[5:5+data_sz])

def main():
    global key
    global nonce
                
    for pkt in rdpcap("../dist/flow.pcapng"):
        if ICMP in pkt:
            ip_pkt:IP = pkt[IP]
            icmp_pkt:ICMP = pkt[ICMP]
            
            if set([ip_pkt.src, ip_pkt.dst]) == set(["192.168.146.133", "192.168.146.130"]):
                #icmp_pkt.show()
                user_data = bytes(icmp_pkt.payload)

                if ip_pkt.src == "192.168.146.133":
                    cmd, payload = split_user_data(user_data)
                    print(f"cmd [{cmd}]")
                    #print(f"payload [{payload}]")
                    data = b""

                    if cmd == 1:
                        # print(len(payload))
                        assert len(payload) == 44
                        nonce = payload[:12]
                        key = payload[12:]

                        data = b"OK"
                    elif cmd == 2 and decryt(payload).startswith(b"fetch cmd"):
                        pass
                    elif cmd == 3:
                        print(bytes([i^0x87 for i in decryt(payload)]))
                else:
                    if not decryt(user_data).startswith(b"OK"):
                        with open("sc", "wb") as fp:
                            print(f"sc len: {len(user_data)}")
                            fp.write(decryt(user_data))
                    


main()