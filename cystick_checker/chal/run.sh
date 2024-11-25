export QEMU_MAC_ADDRESS=52:54:00:12:34:AD
sudo qemu-system-arm -machine mps2-an385 -cpu cortex-m3 \
          -kernel ./src/build/freertos_tcp_mps2_demo.axf \
          -netdev tap,id=mynet0,ifname=virbr0-nic,script=no \
          -net nic,macaddr=$QEMU_MAC_ADDRESS,model=lan9118,netdev=mynet0 \
          -object filter-dump,id=tap_dump,netdev=mynet0,file=/tmp/qemu_tap_dump\
          -display gtk -m 16M  -nographic -serial stdio \
          -monitor null -semihosting -semihosting-config enable=on,target=native