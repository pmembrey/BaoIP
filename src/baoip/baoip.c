#include <stdlib.h>
#include "baoip.h"

baoip_handle_t *baoip_init(unsigned char *myMac, unsigned char *myIP) {

    baoip_handle_t *b;
    // Let's get some RAM!
    b = calloc(1, sizeof(baoip_handle_t));

    // Bounce if we can't allocate...
    if (b == NULL) {
        printf("Couldn't allocate memory for stack structure! Bailing!\n");
        return (0);
    }

    // Copy our MAC in
    memcpy(b->myMAC, myMac, 6);
    // Copy our IP address in
    memcpy(&b->myIP, myIP, 4);


    // Initialise ARP reply...
    arpPacket_t *arpReply = (arpPacket_t *) b->arpReplyRaw;

    // Copy in the source mac
    memcpy(arpReply->ethHeader.sourceMAC, b->myMAC, 6);
    // Set type (0x0806 for ARP)
    arpReply->ethHeader.type[0] = 0x08;
    arpReply->ethHeader.type[1] = 0x06;
    // Hardware Type - 0x0001 Ethernet
    arpReply->hardwareType[0] = 0x00;
    arpReply->hardwareType[1] = 0x01;
    // Protocol type - 0x0800 IP
    arpReply->protocolType[0] = 0x08;
    arpReply->protocolType[1] = 0x00;
    // Hardware size - 0x06
    arpReply->hardwareSize = 0x06;
    // Protocol size - 0x04
    arpReply->protocolSize = 0x04;
    // Opcode - ARP REPLY - 0x0002
    arpReply->opCode[0] = 0x00;
    arpReply->opCode[1] = 0x02;
    // Sender MAC
    memcpy(arpReply->senderMAC, b->myMAC, 6);
    // Sender IP
    memcpy(arpReply->senderIP, &b->myIP, 4);


    // Initialise PING reply...
    pingPacket_t *pingReply = (pingPacket_t *) b->pingReplyRaw;//& pingReplyRaw;

    // Copy in the source mac
    memcpy(pingReply->ethHeader.sourceMAC, b->myMAC, 6);
    // Set type (0x0800 for IPv4)
    pingReply->ethHeader.type[0] = 0x08;
    pingReply->ethHeader.type[1] = 0x00;// Set the IP stuff now - starting with header
    pingReply->ip.versionHeaderLength = 0x45;
    // Set TOS field
    pingReply->ip.tos = 0x00;
    // Set time to live
    pingReply->ip.ttl = 0x40;
    // Set protocol
    pingReply->ip.protocol = 0x01;
    // Set my IP
    memcpy(pingReply->ip.sourceIP, &b->myIP, 4);
    // Set ICMP type - PING REPLY
    pingReply->ping.type = 0x00;
    return (b);
}

int baoip_next_packet(baoip_handle_t *b, unsigned char *packet, int length) {

    ethernetHeader_t *ethernet = (ethernetHeader_t *) packet;
    // Look for ARPs...
    if (ethernet->type[0] == 0x08 && ethernet->type[1] == 0x06) {
        arpPacket_t *arp = (arpPacket_t *) packet;
        if (arp->opCode[1] == 0x01) {

            // @TODO: Replace this with a single integer compare, not a strncmp...
            if (strncmp((char *) &b->myIP, arp->targetIP, 4) == 0) {
                // ARP Request - and they're looking for me!

                // Apply struct to our raw pointer
                arpPacket_t *arpReply = (arpPacket_t *) b->arpReplyRaw;

                // Copy in what we need
                memcpy(arpReply->ethHeader.destinationMAC, arp->senderMAC, 6);
                memcpy(arpReply->targetMAC, arp->senderMAC, 6);
                memcpy(arpReply->targetIP, arp->senderIP, 4);

                // Send it!
                (b->sendFunctionPtr)((unsigned char *) arpReply, sizeof(arpPacket_t));

            }


        }
    }

    if (ethernet->type[0] == 0x08 && ethernet->type[1] == 0x00) {
        //IPv4 data
        ipHeader_t *ip = (ipHeader_t *) (packet + sizeof(ethernetHeader_t));

        if (ip->protocol == 0x01) { // ICMP
            pingPacket_t *ping = (pingPacket_t *) packet;
            if (ping->ping.type == 0x08) { //ICMP Echo request

                // Apply struct to to our raw pointer
                pingPacket_t *pingReply = (pingPacket_t *) b->pingReplyRaw;

                // Copy in what we need
                memcpy(pingReply->ethHeader.destinationMAC, ping->ethHeader.sourceMAC, 6);
                memcpy(pingReply->ip.destinationIP, ping->ip.sourceIP, 4);
                memcpy(pingReply->ping.identifier, ping->ping.identifier, 4);


                // @TODO: This should use the value from the IP header and not assume the length provided is correct
                uint16_t dataLength = length - sizeof(pingPacket_t);

                // Copy the length value
                memcpy(pingReply->ip.totalLength, ping->ip.totalLength, 2);

                // Copy the payload across (have to echo the sender)
                memcpy(((void *) pingReply + sizeof(pingPacket_t)), ((void *) ping + sizeof(pingPacket_t)), dataLength);


                // Send the ping response
                (b->sendFunctionPtr)((unsigned char *) pingReply, sizeof(pingPacket_t) + dataLength);


            }
        }


        return 0;
    }
    return 0;
}

void baoip_set_send_function(baoip_handle_t *b, int (*sendFunctionPtr)(unsigned char *, int length)) {

    b->sendFunctionPtr = sendFunctionPtr;
}