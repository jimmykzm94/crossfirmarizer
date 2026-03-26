import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../crossfirmarizer_client_python/src/crossfirmarizer')))

from serial_packet import create_packet, parse_packet

# arg_payload = "001 013 001"
# arg_encoded_buff = "006 001 002 013 001 015 000"

# try:
#     payloads = [int(x, 10) for x in arg_payload.split()]
#     encoded_buff = [int(x, 10) for x in arg_encoded_buff.split()]
# except:
#     print("Parse error, exit!")
#     exit(1)

# packet = create_packet(payloads[0], bytes(payloads[1:]))
# packets = "".join(f"{int(x):03} " for x in packet)
# print(f"Encoded Packet: {packets}")

# cmd, payload, checksum = parse_packet(bytes(encoded_buff))
# payloads = "".join(f"{int(x):03} " for x in payload)
# print(f"Decode: {cmd:03} {payloads}")

if len(sys.argv) < 3:
    print('Not enough arguments')
    exit(1)

try:
    mode = sys.argv[1]
    if mode == 'encode':
        payloads = [int(x, 10) for x in sys.argv[2].split()]
        packet = create_packet(payloads[0], bytes(payloads[1:]))
        packets = "".join(f"{int(x):03} " for x in packet)
        print(packets)
    elif mode == 'decode':
        encoded_buff = [int(x, 10) for x in sys.argv[2].split()]
        cmd, payload, checksum = parse_packet(bytes(encoded_buff))
        payloads = "".join(f"{int(x):03} " for x in payload)
        print(f"{cmd:03} {payloads}")
    else:
        print('Invalid mode')
        exit(1)
except:
    print("Parse error, exit!")
    exit(1)
