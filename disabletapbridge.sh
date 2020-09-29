#	chmod u+x disabletapbridge.sh
#	sudo ./disabletapbridge.sh

# This is used to re-enable internet access after testing qemu networked code

echo "Run as sudo"
sudo ip link set br0 down
sudo brctl delbr br0
