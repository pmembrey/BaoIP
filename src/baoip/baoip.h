#ifndef BAOIP_H
#define BAOIP_H

#include <stdio.h>
#include <pcap/pcap.h>
#include <netinet/in.h>
#include <string.h>

#define ETH_SIZE 1518

#pragma pack(1)
typedef struct ethernetHeader {
    char destinationMAC[6];
    char sourceMAC[6];
    char type[2];
} ethernetHeader_t;

typedef struct ipHeader {
    char versionHeaderLength;
    char tos;
    char totalLength[2];
    char identification[2];
    char flagsAndFrag[2];
    char ttl;
    char protocol;
    char headerChecksum[2];
    char sourceIP[4];
    char destinationIP[4];
} ipHeader_t;

typedef struct arpPacket {
    ethernetHeader_t ethHeader;
    char hardwareType[2];
    char protocolType[2];
    uint8_t hardwareSize;
    uint8_t protocolSize;
    char opCode[2];
    char senderMAC[6];
    char senderIP[4];
    char targetMAC[6];
    char targetIP[4];
} arpPacket_t;


typedef struct pingPacketHeader {
    char type;
    char code;
    char checksum[2];
    char identifier[2];
    char sequenceNumber[2];
} pingPacketHeader_t;

typedef struct pingPacket {
    ethernetHeader_t ethHeader;
    ipHeader_t ip;
    pingPacketHeader_t ping;
} pingPacket_t;

typedef struct pingPacketWithData {
    ethernetHeader_t ethHeader;
    ipHeader_t ip;
    pingPacketHeader_t ping;
    unsigned char data[1500];
} pingPacketWithData_t;

typedef struct baoip_handle {
    int (*sendFunctionPtr)(unsigned char *, int length);

    unsigned char arpReplyRaw[sizeof(arpPacket_t)];
    unsigned char pingReplyRaw[ETH_SIZE];
    unsigned char myMAC[6];
    uint32_t myIP;
} baoip_handle_t;

#pragma pack()

// Consts (all in big endian to save on ntohs et al
const uint16_t baoip_ARP_TYPE = 1544;
const uint16_t baoip_IP_TYPE = 8;

pcap_t *phandle;
char *device = "enp0s26u1u6";
const u_char *packet;
char errbuf[PCAP_ERRBUF_SIZE];

struct pcap_pkthdr *header;


// Function prototypes
baoip_handle_t *baoip_init(unsigned char *myMac, unsigned char *myIP);

int baoip_next_packet(baoip_handle_t *b, unsigned char *packet, int length);

void baoip_set_send_function(baoip_handle_t *b, int (*sendFunctionPtr)(unsigned char *, int length));

#endif