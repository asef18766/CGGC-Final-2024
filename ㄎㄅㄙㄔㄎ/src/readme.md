# stages
1. rough bytes obsfucated C2
2. send chacha20 encrypted ICMP packets
3. recv shellcode
4. crawel PEB and hash -> get `CreateFile`, `ReadFileEx`
5. create open `flag` and get content

# c2 proto
* 1 bytes step + 4 bytes `len` + `len` bytes payload
## steps
1. victim generate & send chacha20 key -> attacker reply `OK`
2. victim send `fetch cmd` -> attack reply shellcode
3. victim send flag -> attacker reply `OK` -> end