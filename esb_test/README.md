ESB Test Program
===
This README documents the test program esb_test.  The intent of this program is to test the esb user interface between two different Nordic RF wireless devices.

Installing Required Software
---
You will need to download and install the gcc compiler along with the tools that you'll need to run make. You can do this through either Cygwin or MinGW.  The choice is up to you as to which one you want to use.  Here are some links to get you started:
```
https://sourceware.org/cygwin/
http://www.mingw.org/
```
To me, this always seems to be a little bit of a struggle for there isn't one single application installer that seems to do everything that you need.  So it gets to be a little bit of a hunt-and-peck process, but stick with it and you'll eventually get things to work properly.

Building the Test Program
---
Once you can run make and have it call gcc, building the test program is as simple as entering the following command:
```
$ cd <path>/nordic_esb/esb_test
$ make all
gcc  -c -o obj/esb_test.o src/esb_test.c
Compiled obj/esb_test.o successfully!
gcc -o  bin/esb_test obj/esb_test.o
aLinking bin/esb_test complete!
```

Executing Test Program
---
First of all, you will need two different Nordic dongles, part number PCA10031, in order to test the esb interface.

The concept of this test program is very simple.  It transmits out a packet of data and expects to receive the transmitted packet back.  When it does, it then compares the received data with the packet that was sent to verify that the test completed successfully.  It then completes by writing out the results of the test.

To run this test program, you will need two different Nordic dongles, part number PCA10031.  You will need to have one of the dongles loaded with the esb_transceiver firmware and the second dongle will need to be loaded with the echo firmware.  Once it receives the data, it verifies that the returned data was identical to the data that was transmitted and then prints out a passing or failing message.  

What this program does is it first connects to the serial port on which the dongle with the esb_transceiver firmware is loaded. It then generates a packet of data, transmits the data with the nordic transmitter using the esb protocol, and then waits to receive that data back from the dongle on which the echo firmware is loaded.  

To run this test case, you will need to enter the following command:
```
<path>/esb_test -n <comm_port> -v 0 -t 3 -c 1000
```
Here is an explanation of the different user parameters available with this test 
program:
```
-n      Defines the port to use.  Default is COM8
-t      Selects the test to be executed.  Default is 0
            0   Displays any data received (intended for debugging)
            1   Fixed packet size and sets the data to be the same as the 
                index
            2   Random packet size and sets the data to be the same as the 
                index
            3   Random packet size and sets the data to a random values
            4   Fixed packet size and sets the data to random values
-c      Sets the number of test executions to be performed.  Default is 1
-v      Sets the verbosity level.  Default is 0
            0   Minimum output with test results only
            1   Prints out packet information sent/received
            2   Displays the packet data sent/received
-h      Set program to halt on first fail.  Default is 0
            0       Halt is not set
            N > 0   Program halts on first failure
-s      Sets format for data outputted to terminal.  Default is 0
            0       Data is displayed in hex
            N > 0   Data is displayed as a string (not very useful)
-f      Sets the packet size for fixed size tests.  Default is 246
```

Simple Test Execution Example
---
Here is an example run.  In it, test 3 is being executed (random packet size with random data) for 5 test executions:
```
$ ./bin/esb_test -n COM9 -t 3 -c 5
port: COM9, count: 5
Successfully connected to UART on port COM9 at baud rate 115200.
DCB is ready for use.
Running Test 3
Total Executed: 1, Passing: 1, Failing: 0
Total Executed: 2, Passing: 2, Failing: 0
Total Executed: 3, Passing: 3, Failing: 0
Total Executed: 4, Passing: 4, Failing: 0
Total Executed: 5, Passing: 5, Failing: 0
```

And there you have it, a simple way of testing your device.  As an additional debugging option, you should be able to attach a serial port to the dongle program that's running the echo program.  It will display messages when it receives a packet, the size of the packing, and if it was successful in retransmitting the packet. 