/* Bootstrapper for BaoIP
   Written by Peter Membrey
   Adapted with permission from Ian Seyler's minIP project

   Compile & Usage: See README.md
 */


/* Global Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <fcntl.h>

// Include the headers for BaoIP
#include  "src/baoip/baoip.h"


/* Global functions */
int net_init(char *interface);

int net_send(unsigned char *data, int bytes);

int net_recv(unsigned char *data);

/* Global defines */
#undef ETH_FRAME_LEN
#define ETH_FRAME_LEN 1518

/* Global variables */
unsigned char src_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // server address
unsigned char src_IP[4] = {0, 0, 0, 0};
unsigned char src_SN[4] = {0, 0, 0, 0};

unsigned char *abuffer;
int s; // Socket variable
int running = 1, c, retval;
struct sockaddr_ll sa;
struct ifreq ifr;

unsigned int tint0, tint1, tint2, tint3;

int main(int argc, char *argv[]) {
    printf("BaoIP v0.1 (2015-03-07)\n");
    printf("Written by Peter Membrey\n");

    /* first argument needs to be a NIC */
    if (argc < 4) {
        printf("Insufficient arguments!\n");
        printf("%s interface ip subnet\n", argv[0]);
        exit(0);
    }

    /* Parse the IP and Subnet */
    sscanf(argv[2], "%u.%u.%u.%u", &tint0, &tint1, &tint2, &tint3);
    src_IP[0] = tint0;
    src_IP[1] = tint1;
    src_IP[2] = tint2;
    src_IP[3] = tint3;
    sscanf(argv[3], "%u.%u.%u.%u", &tint0, &tint1, &tint2, &tint3);
    src_SN[0] = tint0;
    src_SN[1] = tint1;
    src_SN[2] = tint2;
    src_SN[3] = tint3;

    net_init(argv[1]); // Get us a socket that can handle raw Ethernet frames

    abuffer = (void *) malloc(ETH_FRAME_LEN);

    printf("This host:\n");
    printf("HW: %02X:%02X:%02X:%02X:%02X:%02X\n", src_MAC[0], src_MAC[1], src_MAC[2], src_MAC[3], src_MAC[4], src_MAC[5]);
    printf("IP: %u.%u.%u.%u\n", src_IP[0], src_IP[1], src_IP[2], src_IP[3]);
    printf("SN: %u.%u.%u.%u\n", src_SN[0], src_SN[1], src_SN[2], src_SN[3]);


    // Create the BaoIP handle
    baoip_handle_t *bh;

    // Initialize the stack
    bh = baoip_init(src_MAC, src_IP);
    // Set the send function
    baoip_set_send_function(bh, &net_send);

    // Note: For better latency we "spin" on the socket - it burns a core but knocks off 100us of latency.
    while (running == 1) {
        // Get packet and inject it into BaoIP
        retval = net_recv(abuffer);
        // Got data?
        if (retval > 0) {
            // Inject it into the stack
            baoip_next_packet(bh, abuffer, retval);
        }


    }


    printf("\n");
    close(s);
    return 0;
}


int net_init(char *interface) {
    /* Open a socket in raw mode */
    s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (s == -1) {
        printf("Error: Could not open socket! Check your permissions.\n");
        exit(1);
    }

    /* Which interface are we using? */
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ);

    /* Does that interface exist? */
    if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
        printf("Interface '%s' does not exist.\n", interface);
        close(s);
        exit(1);
    }

    /* Is that interface up? */
    ioctl(s, SIOCGIFFLAGS, &ifr);
    if ((ifr.ifr_flags & IFF_UP) == 0) {
        printf("Interface '%s' is down.\n", interface);
        close(s);
        exit(1);
    }

    /* Configure the port for non-blocking */
    if (-1 == fcntl(s, F_SETFL, O_NONBLOCK)) {
        printf("fcntl (NonBlocking) Warning\n");
        close(s);
        exit(1);
    }


    /* Get the MAC address */
    ioctl(s, SIOCGIFHWADDR, &ifr);
    for (c = 0; c < 6; c++) {
        src_MAC[c] = (unsigned char) ifr.ifr_ifru.ifru_hwaddr.sa_data[c]; // This works... but WTF
    }

    /* Write in the structure again */
    ioctl(s, SIOCGIFINDEX, &ifr);

    /* Configure the rest of what we need */
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_ifindex = ifr.ifr_ifindex;
    sa.sll_protocol = htons(ETH_P_ALL);

    /* We should now have a working port to send/recv raw frames */
    return 0;
}


/* net_send - Send a raw Ethernet packet */
// Wrapper for OS send function
// Returns number of bytes sent
int net_send(unsigned char *data, int bytes) {
    return (sendto(s, data, bytes, 0, (struct sockaddr *) &sa, sizeof(sa)));
}


/* net_recv - Receive a raw Ethernet packet */
// Wrapper for OS recv function
// Returns number of bytes read
int net_recv(unsigned char *data) {
    return (recvfrom(s, data, ETH_FRAME_LEN, 0, 0, 0));
}
