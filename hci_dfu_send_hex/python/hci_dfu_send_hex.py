# Embedded file name: hci_dfu_send_hex.py
import sys, os
import time
import intelhex
from ctypes import c_uint32, c_uint8
from threading import Thread
from serial import Serial
from datetime import datetime, timedelta
import argparse
import crc16pure
VERSION = '1.0'
import getopt

def convert_uint32_to_array(val):
    uint16_data = c_uint32(val)
    byte0 = c_uint8(uint16_data.value & 255)
    byte1 = c_uint8((uint16_data.value & 65280) >> 8)
    byte2 = c_uint8((uint16_data.value & 16711680) >> 16)
    byte3 = c_uint8((uint16_data.value & 4278190080L) >> 24)
    return [byte0.value,
     byte1.value,
     byte2.value,
     byte3.value]


class ThreadedFunction(Thread):

    def __init__(self, func, *args, **kwargs):
        self.func = func
        self.args = args
        self.kwargs = kwargs
        super(ThreadedFunction, self).__init__()

    def run(self):
        self.func(*self.args, **self.kwargs)


def hex_info(hexfile):
    start = hexfile.minaddr()
    end = hexfile.maxaddr() + 1
    size = end - start
    return (size, start, end)


def open_hex(filename):
    try:
        ih = intelhex.IntelHex(filename)
        return ih
    except (IOError, intelhex.IntelHexError) as e:
        print 'Error reading file: %s\n' % e
        raise Exception('Could not read hex format')


def parts_to_four_bytes(seq, dicp, rel, pkt_type, pkt_len):
    ints = [0,
     0,
     0,
     0]
    ints[0] = seq | (seq + 1) % 8 << 3 | dicp << 6 | rel << 7
    ints[1] = pkt_type | (pkt_len & 15) << 4
    ints[2] = (pkt_len & 4080) >> 4
    ints[3] = ~sum(ints[0:3]) + 1 & 255
    return ''.join((chr(b) for b in ints))


def int32_to_bytes(nr):
    ints = [0,
     0,
     0,
     0]
    ints[0] = nr & 255
    ints[1] = (nr & 65280) >> 8
    ints[2] = (nr & 16711680) >> 16
    ints[3] = (nr & 4278190080L) >> 24
    return ''.join((chr(b) for b in ints))


def decode_esc_chars(data):
    """Replace 0xDBDC with 0xCO and 0xDBDD with 0xDB"""
    result = []
    while len(data):
        char = data.pop(0)
        if char == 219:
            char2 = data.pop(0)
            if char2 == 220:
                result.append(192)
            elif char2 == 221:
                result.append(219)
            else:
                raise Exception('Char 0xDB NOT followed by 0xDC or 0xDD')
        else:
            result.append(char)

    return result


def encode_packet(data_in):
    """Replace 0xCO  with 0xDBDC and 0xDB with 0xDBDD"""
    result = []
    data = []
    for i in data_in:
        data.append(ord(i))

    while len(data):
        char = data.pop(0)
        if char == 192:
            result.extend([219, 220])
        elif char == 219:
            result.extend([219, 221])
        else:
            result.append(char)

    return ''.join((chr(i) for i in result))

def print_packet(data):
    print "Sending Packet: " + str(len(data))
    i = 0
    for char in data:
        print "%#x" % ord(char),
        if i % 16 == 15:
            print ""
        i = i + 1
    if i % 16 != 0:
        print ""


DATA_INTEGRITY_CHECK_PRESENT = 1
RELIABLE_PACKET = 1
HCI_PACKET_TYPE = 14
DFU_START_PACKET = 2
DFU_END_PACKET = 4

class HciPacket(object):
    """Class representing a single HCI packet"""
    sequence_number = 0

    def __init__(self, data = ''):
        HciPacket.sequence_number = (HciPacket.sequence_number + 1) % 8
        self.temp_data = ''
        self.temp_data += parts_to_four_bytes(HciPacket.sequence_number, DATA_INTEGRITY_CHECK_PRESENT, RELIABLE_PACKET, HCI_PACKET_TYPE, len(data))
        self.temp_data += data
        crc = crc16pure.crc16xmodem(self.temp_data, crc=65535)
        self.temp_data += chr(crc & 255)
        self.temp_data += chr((crc & 65280) >> 8)
        self.temp_data = encode_packet(self.temp_data)
        self.data = chr(192)
        self.data += self.temp_data
        self.data += chr(192)


class Controller(object):

    def __init__(self):
        self.file_path = None
        self.com_port = None
        self.flow_control = None
        self.baud_rate = None
        return

    def progress_callback(self, value):
        print 'Progress: ', value

    def timeout_callback(self):
        print 'Transmission timeout.'

    def set_filepath(self, filepath):
        self.file_path = filepath
        if not os.path.isfile(os.path.abspath(self.file_path)):
            print 'File does not exist: %s' % os.path.abspath(self.file_path)
            sys.exit(1)

    def upload_file(self):
        if not self.file_path:
            raise Exception('File path not set, or not found.')
        self.progress_callback(0)
        s = ThreadedFunction(self.upload_firmware)
        s.start()

    def upload_firmware(self):

        def percentage(part, whole):
            return int(100 * float(part) / float(whole))

        def get_ack_nr(uart):

            def is_timeout(start_time, timeout_sec):
                return not datetime.now() - start_time <= timedelta(0, timeout_sec)

            uart_buffer = []
            start = datetime.now()
            while uart_buffer.count(192) < 2:
                temp = uart.read(1)
                #if len(temp) > 0:
                #    print hex(ord(temp))
                #else:
                #    print "''"
                if temp:
                    uart_buffer.append(ord(temp))
                if is_timeout(start, 5):
                    HciPacket.sequence_number = 0
                    self.timeout_callback()
                    break

            data = decode_esc_chars(uart_buffer)
            data = data[1:-1]
            return data[0] >> 3 & 7

        bin_image = open_hex(self.file_path).tobinstr()
        #print_packet(bin_image)
        start_data = int32_to_bytes(DFU_START_PACKET)
        start_data += int32_to_bytes(len(bin_image))
        #print "Binary Length: " + str(hex(len(bin_image)))
        stop_data = int32_to_bytes(DFU_END_PACKET)
        packets = []
        packet = HciPacket(start_data)
        packets.append(packet)
        for i in range(0, len(bin_image), 48):
            data_packet = HciPacket(int32_to_bytes(3) + bin_image[i:i + 48])
            packets.append(data_packet)

        packet = HciPacket(stop_data)
        packets.append(packet)
        uart = None
        try:
            uart = Serial(self.com_port, self.baud_rate, rtscts=self.flow_control, timeout=1)
        except Exception as e:
            print 'UART could not be opened on %s' % self.com_port
            print e

        try:
            if uart:
                for seq, pkt in enumerate(packets[:-1]):
                    attempts = 0
                    last_ack = None
                    packet_sent = False
                    while packet_sent == False:
                        #print_packet(pkt.data)
                        uart.write(pkt.data)
                        attempts += 1
                        ack = get_ack_nr(uart)
                        #print "ack:" + str(ack)
                        if last_ack == None:
                            last_ack = ack
                            break
                        if ack == (last_ack + 1) % 8:
                            last_ack = ack
                            packet_sent = True
                        if attempts > 3:
                            raise Exception('Three failed tx attempts encountered on packet {0}'.format(seq))

                    self.progress_callback(percentage(seq, len(packets)))

                time.sleep(1)
                #print_packet(packets[-1].data)
                uart.write(packets[-1].data)
                self.progress_callback(percentage(len(packets), len(packets)))
                uart.close()
        except IndexError as e:
            print 'Ack out of sequence, or no ack returned'

        return


def main(argv, arglen):
    print ''


if __name__ == '__main__':
    controller = Controller()
    parser = argparse.ArgumentParser(description='Hex File of firmware to be upgraded.')
    parser.add_argument('--file', '-f', type=str, required=True, dest='file', help='Filename of Hex file.')
    parser.add_argument('--port', '-p', type=str, required=True, dest='comport', help='COM Port to which the device is connected.')
    parser.add_argument('--flowcontrol', '-fc', action='store_true', required=False, dest='flowcontrol_bool', help='Enable flow control, default: disabled.')
    parser.add_argument('--baudrate', '-b', type=int, required=False, default=38400, dest='baudrate', help='Desired baudrate 38400/96000/115200, default: 38400.')
    args = parser.parse_args()
    print 'Sending file {0} to {1}, flow control = {2}'.format(args.file, args.comport, args.flowcontrol_bool)
    controller.com_port = args.comport.strip()
    controller.flow_control = args.flowcontrol_bool
    controller.baud_rate = args.baudrate
    controller.set_filepath(args.file.strip())
    controller.upload_file()

