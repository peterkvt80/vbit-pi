/** packet.h
 * VBIT on Raspberry Pi
 * header file for packet.c
 * 
 * Peter Kwan, Copyright 2013
 */

#ifndef _PACKET_H_
#define _PACKET_H_
#define PACKETSIZE 45
#include <stdio.h>
#include <stdint.h>
#include "tables.h"
#include <string.h>
#include <stdlib.h>     /* strtol */

/** copyOL - Copy Output Line
 */
uint8_t copyOL(char *packet, char *textline);

/** copyFL - Copy the Fastext links
 */
void copyFL(char *packet, char *textline, unsigned char mag);

/** dumpPacket - Display the packet in lovely hexadecimal
 */
void dumpPacket(char* packet);

/** Also need to define a field of ...
 */

/** Check that parity is correct for the packet payload
 * The parity is set to odd for all bytes from offset to the end
 * Offset should be at least 3, as the first three bytes have even parity
 * The bits are then all reversed into transmission order
 * \param packet : packet to check
 * \param offset : start offset to check. (5 for rows, 13 for header)
 */ 
void Parity(char *packet, uint8_t offset);

/** PacketClear
 * \param packet : Byte array at least PACKETLENGTH bytes;
 * \param value : set all the bytes to this value
 */
void PacketClear(uint8_t *packet, uint8_t value);

/** PacketQuiet - Set the entire packet to 0. Remember not to do parity after this.
 * \param packet : Byte array at least PACKETLENGTH bytes;
 */
void PacketQuiet(uint8_t *packet);


/** PacketPrefixValue
 * Call this when starting a new packet.
 * \param packet : Byte array at least PACKETSIZE bytes;
 * \param mag : Magazine number 1..8
 * \param row : Row number 0..31
 * \param value : Value to fill the packet with 
 */
void PacketPrefixValue(uint8_t *packet, uint8_t mag, uint8_t row, int value);

/** PacketPrefix
 * Call this when starting a new packet.
 * Same as PacketPrefixValue with value set to 0.
 * \param packet : Byte array at least PACKETLENGTH bytes;
 * \mag : Magazine number 1..8
 * \row : Row number 0..31
 */
void PacketPrefix(uint8_t *packet, uint8_t mag, uint8_t row);
void PacketHeader(char *packet ,unsigned char mag, unsigned char page, unsigned int subcode,
			unsigned int control, char *caption);

extern volatile uint32_t UTC; // 10:00am

#endif

