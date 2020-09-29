Introduction:

By default, the xv6 operating system is a lonesome and isolated environment. There is no native network support to connect your system to the outside world, thus ensuring a deep sense of FOMO as you sacrifice your social life for CS3210 coursework.
Enter xv6Chat: an IRC-inspired group chat application for xv6 that runs over TCP/IP. Simply launch an xv6Chat server from the shell to create an open chatroom that other xv6 users in your OS class will be able to connect to given a specified port and address.
Of course, xv6Chat is pointless without any OS support for a network connection. That is why we are aiming to implement a full network stack for xv6 for xv6Chat to run ontop of.  


Background:

Internet Relay Chat (IRC) was created in 1988. Its purpose was to allow multi-user communication on bulletin-board systems, replacing the previously used MultiUser Talk (MUT) program. IRC uses a client-server model, where clients connected to a server send single-line messages, which are then dispatched by the server. Communication to groups of users over IRC is managed with channels. Users can also send commands via IRC, which are used to, among many other things, list or join available channels on the network.
Problem
Our team aims to expand on the network interface card driver and link layer protocol (ARP) built for the xv6 in Lab 5 by adding an internet layer protocol (IP) and a transport layer protocol (TCP). Once the network stack is established, we will implement an application layer protocol to function similarly to IRC that works on a client-server networking model.
xv6Chat should allow multiple people to connect to the chatroom server where anything that any user types gets transmitted to the other users. To accomplish this, we must be able to effectively communicate with multiple sockets at once, send and receive on the same socket at the same time, and implement IP addressing.


Approach:

Configuring QEMU:
The first necessary step will be to tweak some configurations for QEMU’s virtual network. We will need to enable the use of its user-mode network stack as it requires no administrative privileges to run. Additionally, routing data from Internet connections between our host machine and virtual network will require running a server on some port of the host machine that connects through to some xv6 port.

Porting lwIP:

We plan to leverage the open-source lwIP suite for porting a minimal TCP/IP stack to the xv6 kernel as a network server. lwIP provides an abstract operating system emulation layer for maximum portability that will require our own implementation specific to the xv6 operating system. This will require adding some xv6 system services for timers, process synchronization, and message passing mechanisms. 
Extending system calls
Send (TX) and Receive (RX) system calls will need to be implemented to allow our E1000 driver to interface with the network server. The former will enable packet transmissions from user space, and the latter will receive a packet from the E1000 and expose it to user space. Additionally, a new system call will need to be added to give user space access to the kernel timer, which is maintained by hardware clock interrupts.

User environment interface:

Both server and client user programs for xv6Chat will be designed to make use of the network server’s BSD socket interface via inter-process communication (IPC) messages. The lwIP source includes a familiar file descriptor-based sockets API for users through functions found in `lwip/sockets.h`. This framework should simplify the use of TCP/IP sockets on xv6 to be no different than any standard Linux server-client model application.


Potential Problems & Issues:

Multithreading:

In its current state, the lwIP suite is not completely threadsafe. Specifically, in its current state, there are ways to trick two applications into using the same socket, which should not be possible. Solving this problem may be tricky and will certainly involve moving some processing around within the suite, and potentially deeper into the kernel itself.
This may not necessarily impact our implementation of xv6Chat, since we will likely only have one socket-using process per machine for the sake of demonstration, yet we will be keeping this in mind as we develop the network stack.

IRC Complexity:

The full implementation of IRC (irc2.4.0 defined by RFC#1459) is a complex,  	fully-featured protocol. xv6Chat may end up being a pared-down version of this protocol in the interest of time, depending on the ultimate difficulty in which lwIP and the rest of the network stack is ported/developed.
