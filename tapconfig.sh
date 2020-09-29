# $ sudo apt install uml-utilities bridge-utils
# $ chmod u+x tapconfig.sh
# $ sudo ./tapconfig.sh

tunctl -u $SUDO_USER
echo 1 > /proc/sys/net/ipv4/conf/tap0/proxy_arp
sudo ip link set tap0 up

tunctl -u $SUDO_USER
echo 1 > /proc/sys/net/ipv4/conf/tap1/proxy_arp
sudo ip link set tap1 up

tunctl -u $SUDO_USER
echo 1 > /proc/sys/net/ipv4/conf/tap2/proxy_arp
sudo ip link set tap2 up

tunctl -u $SUDO_USER
echo 1 > /proc/sys/net/ipv4/conf/tap3/proxy_arp
sudo ip link set tap3 up

brctl addbr br0
brctl addif br0 tap0
brctl addif br0 tap1
brctl addif br0 tap2
brctl addif br0 tap3

#brctl addif br0 enp0s3
brctl addif br0 enp0s8

ip link set br0 up


# Note that attaching the VirtualBox enp0s3 interface to
# the bridge b0 will disable Internet connectivity for
# your virtual machine. To re-enable, run these commands:
#
#	sudo ip link set br0 down
# 	sudo brctl delbr br0
