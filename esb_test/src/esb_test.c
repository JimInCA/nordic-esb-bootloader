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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define MIN_LENGTH 1
#define MAX_LENGTH 246

HANDLE uart;

const uint8_t tag[4] = {'L', 'E', 'A', 'P'};

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

int32_t main(int32_t argc, uint8_t **argv)
{
    uint8_t *portname = "COM8";
    int32_t i, j, index, error, eeh, loop;
    int32_t length, checksum;
    int32_t done;
    uint32_t test_count;

    uint32_t count = 0;
    uint32_t string = 0;
    uint32_t test = 0;
    uint32_t halt = 0;
    uint32_t fixed_length = MAX_LENGTH;
    uint32_t verbose = 0;

    int32_t attempt;
    uint32_t rdlen, wrlen;
    uint8_t rd_buf[256];
    uint8_t wr_buf[256];

    // simple command line parser
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0) {
            portname = argv[++i];
        }
        else if (strcmp(argv[i], "-c") == 0) {
            count = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            string = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-t") == 0) {
            test = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0) {
            halt = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-v") == 0) {
            verbose = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "-f") == 0) {
            fixed_length = atol(argv[++i]);
            if (fixed_length > MAX_LENGTH)
                fixed_length = MAX_LENGTH;
            else if (fixed_length < MIN_LENGTH)
                fixed_length = MIN_LENGTH;
        }
        else {
            printf("Unknown input paramter: %s\n", argv[i]);
            return -1;
        }
    }

    printf("port: %s, count: %d\n", portname, count);

    if (init_uart(portname, 115200) != 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }

    if (test == 0)
    {
        do {
            if (ReadFile(uart, rd_buf, sizeof(rd_buf), &rdlen, NULL)) {
                if (string > 0) {
                    rd_buf[rdlen] = 0;
                    printf("%s", rd_buf);
                } else { 
                    unsigned char   *p;
                    printf("Read %d:", rdlen);
                    for (p = rd_buf; rdlen-- > 0; p++)
                        printf(" 0x%x", *p);
                    printf("\n");
                }
            } else if (rdlen < 0) {
                printf("Error from read: %d: %s\n", rdlen, strerror(errno));
            }
            // repeat read to get full message
        } while (--count > 0);
    } else if (test < 5) {
        // test one sets the data to be the same as the index
        // test two sets the data to be the same as the index but generates random packet size
        // test three sets both the packet size and the data to a random value
        printf("Running Test %d\n", test);

        // initialize random number generator
        srand((uint32_t)time(NULL));

        // initialize write buffer
        wr_buf[0] = tag[0];
        wr_buf[1] = tag[1];
        wr_buf[2] = tag[2];
        wr_buf[3] = tag[3];
        wr_buf[4] = (uint8_t)fixed_length;    // can not go greater than 0xf7
        wr_buf[5] = 0x00;

        eeh = 0;
        loop = count;
        test_count = 1;
        do {
            char random_length;
            do {
                random_length = (char) rand();
            } while ((random_length < MIN_LENGTH) || (random_length > MAX_LENGTH));
            wr_buf[4] = ((test == 1) || (test ==4)) ? (uint8_t)fixed_length : random_length;
            for (j = 0x00, i = 6; i < (6+wr_buf[4]); j++, i++) {
                wr_buf[i] = test < 3 ? j : (char) rand();
                rd_buf[i] = 0;
            }

            // write the output buffer to the uart
            if (!WriteFile(uart, wr_buf, (DWORD)i, &wrlen, NULL))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                    printf("UART failed to write.");
            }
            if (verbose > 0)
                printf("Transmitted %d bytes\n", wrlen);

            index = length = done = 0;
            do {
                // we want to start by reading one character at a time and checking
                // for the first six characters in the header.
                attempt = 0;
                do {
                    ReadFile(uart, &rd_buf[index], 1, &rdlen, NULL);
                } while ((rdlen == 0) && (++attempt < 10));
                if (rdlen == 0) {
                    printf("Device not responding.\n");
                    break;
                }
                else if (rdlen < 0) {
                    printf("Error from read: %d: %s\n", rdlen, strerror(errno));
                }
                else if (rdlen > 0)
                {
                    switch (index)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3: if (rd_buf[index] == tag[index])
                                index++;
                            else
                            {
                                index = 0;
                                break;
                            }
                            break;
                    case 4: length = rd_buf[index];
                            index++;
                            break;
                    case 5: checksum = rd_buf[index];
                            index++;
                            break;
                    }
                    if (index == 6)
                    {
                        // at this point, we've received the header, so now lets read
                        // in all of the data based on length
                        do {
                            ReadFile(uart, &rd_buf[index], sizeof(rd_buf), &rdlen, NULL);
                            length -= rdlen;
                            index += rdlen;
                        } while (length > 0);
                        done = 1;
                    }
                }
            } while (!done);
            
            // write out data that was read from rf
            if (verbose > 0)
                printf("Received %d bytes\n", index);

            i = error = 0;
            do {
                if (wr_buf[i] != rd_buf[i])
                {
                    printf ("ERROR: wr_buf[%d]:0x%02x != rd_buf[%d]:0x%02x\n", i, wr_buf[i], i, rd_buf[i]);
                    error += 1;
                }
                i++;
            } while (error == 0 && i < index);
            if (verbose > 0)
                printf("Test %d/%d %s\n", test, test_count, error == 0 ? "Passed":"Failed");

            if ((error != 0) || (verbose > 1))
            {
                printf("Write Buffer Data\n");
                for (i = 0; i < index; i++) {
                    printf(" 0x%02x", wr_buf[i]);
                    if (i % 16 == 15)
                        printf("\n");
                }
                if (index % 16 != 0)
                    printf("\n");
                printf("Read Buffer Data\n");
                for (i = 0; i < index; i++) {
                    printf(" 0x%02x", rd_buf[i]);
                    if (i % 16 == 15)
                        printf("\n");
                }
                if (index % 16 != 0)
                    printf("\n");
            }
            if (error > 0)
                eeh++;
            if ( (eeh > 0) && (halt > 0) )
                break;
            printf("Total Executed: %d, Passing: %d, Failing: %d\n", test_count, test_count - eeh, eeh);
            fflush(stdout);
            test_count++;
        } while (--loop > 0);
    }

    CloseHandle(uart);

    return 0;
}

