/*
 * Copyright(c) 2019 - Jim Newman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:

 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* # Embedded file name: hci_dfu_send_hex.py
 */

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "crc16pure.h"

#define DATA_INTEGRITY_CHECK_PRESENT 1
#define RELIABLE_PACKET   1
#define HCI_PACKET_TYPE  14
#define DFU_INIT_PACKET   1
#define DFU_START_PACKET  2
#define DFU_DATA_PACKET   3
#define DFU_END_PACKET    4

// Linux seems to only support a packet size of 48
//#define PACKET_SIZE 48
#define PACKET_SIZE 512

unsigned long verbose = 0;

HANDLE uart;


int16_t init_uart(uint8_t *serial_port, uint32_t baud)
{
    COMMTIMEOUTS timeouts = { 0 };

    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 100;

    uint8_t adjusted_serial_port[16];

    sprintf(adjusted_serial_port, "\\\\.\\%s", serial_port);
    uart = CreateFile(adjusted_serial_port,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (uart == INVALID_HANDLE_VALUE)
    {
        printf("Failed to connet to UART on port %s\n", serial_port);
        return 1;
    }
    else
    {
        printf("Successfully connected to UART on port %s at baud rate %d.\n", serial_port, baud);
    }

    DCB dcb = { 0 };

    dcb.DCBlength = sizeof(dcb);

    BOOL ret = GetCommState(uart, &dcb);

    if (ret == FALSE)
    {
        ret = GetLastError();
        printf("Error getting current DCB settings: %d\n", ret);
    }
    else
        printf("DCB is ready for use.\n");

    FillMemory(&dcb, sizeof(dcb), 0);
    if (!GetCommState(uart, &dcb))     // get current DCB
    {
        printf("Error in GetCommState.\n");
        return 1;
    }

    // Update DCB rate.
    dcb.BaudRate = baud;

    // Set new state.
    if (!SetCommState(uart, &dcb))
    {
        printf("Error in SetCommState. Possibly a problem with the communications,\n");
        printf("port handle, or a problem with the DCB structure itself.\n");
        return 1;
    }

    if (!SetCommTimeouts(uart, &timeouts))
        printf("Error setting time-outs.\n");

    return 0;
}

uint8_t char_2_uint8(uint8_t ch)
{
    if ( (ch >= '0') && (ch <= '9') )
        return (ch - '0');
    else if ( (ch >= 'A') && (ch <= 'F') )
        return (ch - 'A' + 0x0a);
    else if ( (ch >= 'a') && (ch <= 'f') )
        return (ch - 'a' + 0x0a);
    else
        printf("Error in character conversion.\n");
    return 0;
}


void parts_to_four_bytes(uint8_t *data,
                         uint16_t seq,
                         uint16_t dicp,
                         uint16_t rel,
                         uint16_t pkt_type,
                         uint16_t pkt_len)
{
    data[0] = seq | (seq + 1) % 8 << 3 | dicp << 6 | rel << 7;
    data[1] = pkt_type | (pkt_len & 15) << 4;
    data[2] = (pkt_len & 4080) >> 4;
    data[3] = ~(data[0] + data[1] + data[2]) + 1 & 255;
}

static uint16_t sequence_number = 0;

uint16_t HciPacket(uint8_t *output_array, uint8_t *input_array, uint16_t input_length, uint8_t packet_type)
{
    uint16_t i, j;
    uint8_t temp_array[600];
    uint16_t chksum;

    memset(temp_array, 0x00, sizeof(temp_array));
    sequence_number = (sequence_number + 1) % 8;
    parts_to_four_bytes(temp_array,
                        sequence_number,
                        DATA_INTEGRITY_CHECK_PRESENT,
                        RELIABLE_PACKET,
                        HCI_PACKET_TYPE,
                        input_length+4);
    temp_array[4] = packet_type;
    for (i = 8, j = 0; j < input_length; i++, j++)
        temp_array[i] = input_array[j];
    chksum = crc16xmodem(temp_array, input_length + 8, 0xffff);
    temp_array[i++] = (uint8_t)(chksum & 0x00ff);
    temp_array[i++] = (uint8_t)((chksum & 0xff00) >> 8);

    output_array[0] = 0xC0;
    for (i = 1, j = 0; j < (input_length + 10); j++) {
        if (temp_array[j] == 0xC0) {
            output_array[i++] = 0xDB;
            output_array[i++] = 0xDC;
        } else if (temp_array[j] == 0xDB) {
            output_array[i++] = 0xDB;
            output_array[i++] = 0xDD;
        } else {
            output_array[i++] = temp_array[j];
        }
    }
    output_array[i++] = 0xC0;
    return i;
}


void send_packet(HANDLE uart, uint8_t *packet, int16_t length)
{
    int16_t i;
    uint32_t wr_len;

    //write(uart, packet, length);  // JTN
    WriteFile(uart, packet, length, &wr_len, NULL);
    if (verbose > 0) {
        printf("Sending Packet: %d\n", length);
        for (i = 0; i < length; i++) {
            printf("0x%x ", packet[i]);
            if (i % 16 == 15)
                printf("\n");
        }
        printf("\n");
    }
}


int8_t get_acknowledgement(HANDLE uart)
{
    int16_t i, rdlen, total_read;
    uint8_t rd_buf[16];
    int8_t ack;
    int16_t count;
    uint32_t rd_len;

    rdlen = total_read = count = 0;
    do {
        // rdlen += read(uart, &rd_buf[rdlen], 1);  // JTN
        ReadFile(uart, &rd_buf[rdlen], 1, &rd_len, NULL);
    } while ((rdlen < 6) && (++count < 10));
    if (rdlen != 6)
        return -1;
    if ((rd_buf[0] != 0xC0) || (rd_buf[5] != 0xC0))
        return -1;
    // I need to decode the values read in from device.
    ack = rd_buf[1] >> 3 & 0x07;
    if (verbose > 0) {
        for (i = 0; i < rdlen; i++)
            printf("0x%02x ", rd_buf[i]);
        printf("\n");
        printf("ack: %d\n", ack);
    }

    return ack;
}


int main(int argc, char **argv)
{
    char *portname = NULL;
    char *file_name = NULL;

    unsigned long baudrate = 0;
    unsigned long flowcontrol = 0;
    unsigned long help = 0;

    FILE *fp;
    int16_t i, j, k;
    uint32_t file_size;
    char in_str[64];
    char *bin_out;
    uint8_t checksum;
    uint16_t bin_cnt;
    uint16_t packet_size;
    int8_t ack;
    uint16_t attempt;

    struct record_s {
        uint8_t  reclen;
        uint16_t load_offset;
        uint8_t  rectyp;
        uint8_t  data[256];
        uint8_t  chksum;
    } record;

    uint8_t packet[600];

    // simple command line parser
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-f") == 0) {
            file_name = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0) {
            portname = argv[++i];
        }
        else if (strcmp(argv[i], "-b") == 0) {
            baudrate = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-v") == 0) {
            verbose = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0) {
            help = 1;
        }
        else {
            printf("Unknown input paramter: %s\n", argv[i]);
            return -1;
        }
    }

    if ((help == 1) || (file_name == NULL) || (portname == NULL)) {
        int result = 0;
        printf("usage: hci_dfu_send_hex [-h] -f FILE -p COMPORT [-b BAUDRATE]\n\n");
        if (help == 1) {
            printf("Hex File of firmware to be upgraded.\n\n");
            printf("arguments:\n");
            printf("  -h             Show this help message and exit.\n");
            printf("  -f FILE        Filename of Hex file.\n");
            printf("  -p COMPORT     COM Port to which the device is connected.\n");
            printf("  -b BAUDRATE    Desired baudrate 38400/96000/115200, default: 38400.\n");
        }
        else {
            if (file_name == NULL)
                printf("hci_dfu_send_hex: error: argument -f FILE is required.\n");
            if (portname == NULL)
                printf("hci_dfu_send_hex: error: argument -p COMPORT is required.\n");
            result = -1;
        }
        return result;
    }

    printf("Sending file %s to port %s at baudrate %ld\n", file_name, portname, baudrate);

    if (init_uart(portname, 115200) != 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }

    // let's open the file and check for errors
    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("Error while opening file %s.\n", file_name);
        return -1;
    }

    // find out the file size for later reference
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    //printf("File Size: %d\n", file_size);

    bin_out = malloc(file_size/2);
    if (bin_out == NULL) {
        printf("Error while allocate memory for output file.\n");
        return -1;
    }

    // time to parse the input file
    // The input file is encoded in what's known as IntelHex Format.  This is a format
    // where all of the binary data is convered into ascii.  This ascii data is then
    // included in various types of packets, where each packet is in a single string.
    // What we need to do here is read in each string of data, convert the ascii data
    // back to binary, and then save the binary data is a structure that we can parse.
    file_size = 0;
    while ( fgets(in_str, 128, fp) != NULL ) {
        checksum = 0;
        // the first thing that I need to do is convert the ascii characters into hex digits
        record.reclen = (char_2_uint8(in_str[1]) << 4) | char_2_uint8(in_str[2]);
        record.load_offset =   (char_2_uint8(in_str[3]) << 12) | (char_2_uint8(in_str[4]) << 8)
                             | (char_2_uint8(in_str[5]) << 4 ) | char_2_uint8(in_str[6]);
        record.rectyp = (char_2_uint8(in_str[7]) << 4) | char_2_uint8(in_str[8]);
        for (i = 0, j = 9; i < record.reclen; i++, j+=2)
            record.data[i] = (char_2_uint8(in_str[j]) << 4) | char_2_uint8(in_str[j+1]);
        record.chksum = (char_2_uint8(in_str[j]) << 4) | char_2_uint8(in_str[j+1]);
        // verify check-sum
        checksum = record.reclen;
        checksum += (uint8_t)((record.load_offset & 0xff00) >> 8);
        checksum += (uint8_t)(record.load_offset & 0x00ff);
        checksum += record.rectyp;
        for (i = 0; i < record.reclen; i++)
            checksum += record.data[i];
        checksum = (checksum ^ 0xff) + 1;
        if (checksum != record.chksum) {
            printf("Error when verifing check-sum.\n");
            return -1;
        }
        // now I need to transfer the data in record.data[] to the output file
        if (record.rectyp == 0x00) {
            for (i = 0; i < record.reclen; i++) {
                bin_out[file_size++] = record.data[i];
            };
        };
    }

    // send start packet
    packet[0] = (uint8_t)((file_size & 0x000000ff) >> 0);
    packet[1] = (uint8_t)((file_size & 0x0000ff00) >> 8);
    packet[2] = (uint8_t)((file_size & 0x00ff0000) >> 16);
    packet[3] = (uint8_t)((file_size & 0xff000000) >> 24);
    k = HciPacket(&packet[0], &packet[0], 4, DFU_START_PACKET);
    attempt = 0;
    do {
        send_packet(uart, packet, k);
        ack = get_acknowledgement(uart);
    } while ((ack < 0) && (++attempt < 3));
    if (ack < 0) {
        printf("Error reading acknowledgement from device.\n");
        return -1;
    }

    // send data packets by breaking the input file into smaller more
    // manageable blocks
    bin_cnt = 0;
    while (bin_cnt < file_size) {
        memset(packet, 0x00, sizeof(packet));
        if ((file_size - bin_cnt) < PACKET_SIZE)
            packet_size = file_size - bin_cnt;
        else
            packet_size = PACKET_SIZE;

        k = HciPacket(&packet[0], &bin_out[bin_cnt], packet_size, DFU_DATA_PACKET);
        bin_cnt += packet_size;

        printf("Progress:  %d\n", bin_cnt * 100 / file_size);
        send_packet(uart, packet, k);
        ack = get_acknowledgement(uart);
        if (ack < 0) {
            printf("Error reading acknowledgement from device.\n");
            return -1;
        }
    }

    // send stop packet
    k = HciPacket(&packet[0], &packet[0], 0, DFU_END_PACKET);
    send_packet(uart, packet, k);
    ack = get_acknowledgement(uart);
    if (ack < 0) {
        printf("Error reading acknowledgement from device.\n");
        return -1;
    }

    free(bin_out);
    fclose(fp);
    CloseHandle(uart);

    return 0;
}

