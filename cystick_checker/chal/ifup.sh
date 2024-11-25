export VM_IP_ADDRESS=10.211.55.1
export VM_CIDR=24

sudo ip tuntap add dev virbr0-nic mode tap
sudo ip addr add $VM_IP_ADDRESS/$VM_CIDR dev virbr0-nic
sudo ip link set dev virbr0-nic up