# Socket Programming Cheat Sheet
Sockets have been implemented frequently with Windows WinSock programming. Much of the code in Windows will work similarly to Linux.

Make sure you specify which WinSock version you wish to use:  
#define SCK_VERSION2 0x0202

# Necessary Header Files
## Linux Headers
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <netinet/ip.h> (superset of previous)  
#include <arpa/inet.h>  
#include <netdb.h>  
## Windows Headers
#include <windows.h>

# Arguments for each type of socket
tcp_socket = socket(AF_INET, SOCK_STREAM, 0);  
udp_socket = socket(AF_INET, SOCK_DGRAM, 0);  
raw_socket = socket(AF_INET, SOCK_RAW, protocol);  

tcp ensures both the client and server are connected by verifying the delivery
of the packets between them and as such will reliably send/receive files whereas
udp does not verify the data and the files being sent may get corrupted.  
For version control, tcp should be used as udp does not guarentee that old files will be sent properly to the user.

# C Structs
struct addrinfo {  
- int              ai_flags;  
- int              ai_family;  
- int              ai_socktype;  
- int              ai_protocol;  
- socklen_t        ai_addrlen;  
- struct sockaddr \*ai_addr;  
- char            \*ai_canonname;  
- struct addrinfo \*ai_next;  
};  

The following structs are not as important as all of their information is  
encapsulated by addrinfo after calling getaddrinfo().

struct sockaddr_in {  
- short sin_family; (address family)  
- in_port_t sin_port; (port in network byte order)  
- struct in_addr sin_addr; (ip address)  

struct in_addr {  
uint32_t s_addr; (address in network byte order)  
};

# Functions
**getaddrinfo**(address, port number, addrinfo \* hints, addrinfo \*\* res)  
Gets the information from the given address and socket criteria in hints and stores the results in res

## Socket Functions
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen); - binds addr to sockfd

int listen(int sockfd, int backlog)  
marks the socket to be used to accept information using accept()  
The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
If  a connection request arrives when the queue is full, the client may receive an error with an indication of
ECONNREFUSED or, if the underlying protocol supports retransmission, the request may  be  ignored  so  that  a
later reattempt at connection succeeds.

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)  
Waits for a connection and wakes when a connection is established, usually used when coding the server part.  
The argument sockfd is a socket that has been created with socket(), bound to a local address with bind(),
and is listening for connections after a listen()  
On success, these system calls return a nonnegative integer that is a file descriptor for the accepted socket.
On error, -1 is returned

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) - initiate a connection on a socket  
Returns 0 on success and -1 on error.  

sockets can be closed with close() like any other file descriptor.

## Network Functions
uint16_t htons(uint16_t hostshort) - converts "host byte order" to "network byte order"  
uint16_t ntohs(uint16_t hostshort) - vice versa   

int inet_aton(char *cp, struct in_addr *inp)  
Converts the Internet host address cp from the IPv4
numbers-and-dots notation into binary form (in network byte order) and stores it in the structure that inp points to.  
Returns 1 if the supplied string was successfully interpreted, or 0 if the string is invalid


