import struct

def cobs_encode(data: bytes) -> bytes:
    """Encodes a byte string using Consistent Overhead Byte Stuffing (COBS)."""
    encoded = bytearray()
    encoded.append(0)  # Header byte placeholder
    code_idx = 0
    code = 1
    
    for b in data:
        if b == 0:
            encoded[code_idx] = code
            code = 1
            code_idx = len(encoded)
            encoded.append(0)
        else:
            encoded.append(b)
            code += 1
            if code == 255:
                encoded[code_idx] = code
                code = 1
                code_idx = len(encoded)
                encoded.append(0)
                
    encoded[code_idx] = code
    return bytes(encoded)

def cobs_decode(encoded: bytes) -> bytes:
    """Decodes a COBS-encoded byte string."""
    decoded = bytearray()
    idx = 0
    
    while idx < len(encoded):
        code = encoded[idx]
        if code == 0:
            raise ValueError("Invalid COBS encoding: code byte cannot be zero.")
        
        idx += 1
        for i in range(code - 1):
            if idx >= len(encoded):
                raise ValueError("Invalid COBS encoding: code byte exceeds remaining data.")
            decoded.append(encoded[idx])
            idx += 1
        
        if code < 255 and idx < len(encoded):
            decoded.append(0)
    
    return bytes(decoded)

def create_packet(cmd: int, payload: bytes) -> bytes:
    """Builds a COBS-encoded serial packet with a 0x00 delimiter."""
    length = len(payload)
    
    # Calculate XOR checksum
    checksum = cmd ^ length
    for byte in payload:
        checksum ^= byte
        
    # Pack the unencoded data: CMD + LEN + PAYLOAD + CHK
    fmt = f'<BB{length}sB'
    raw_data = struct.pack(fmt, cmd, length, payload, checksum)
    
    # Encode with COBS and append the frame delimiter
    encoded_data = cobs_encode(raw_data)
    return encoded_data + b'\x00'

def parse_packet(encoded_packet: bytes):
    """Parses a COBS-encoded packet and returns the command, payload, and checksum."""
    if not encoded_packet.endswith(b'\x00'):
        raise ValueError("Packet must end with a 0x00 delimiter.")
    
    # Remove the trailing 0x00 and decode
    encoded_data = encoded_packet[:-1]
    raw_data = cobs_decode(encoded_data)
    
    if len(raw_data) < 3:
        raise ValueError("Decoded packet is too short to be valid.")
    
    cmd = raw_data[0]
    length = raw_data[1]
    
    if len(raw_data) != 2 + length + 1:
        raise ValueError("Decoded packet length does not match expected length.")
    
    payload = raw_data[2:2+length]
    checksum = raw_data[-1]
    
    # Verify checksum
    calculated_checksum = cmd ^ length
    for byte in payload:
        calculated_checksum ^= byte
        
    if checksum != calculated_checksum:
        raise ValueError("Checksum mismatch.")
    
    return cmd, payload, checksum

# Example Usage:
if __name__ == "__main__":
    command_id = 0x01
    
    # Notice we include 0x00 in the payload to prove COBS removes it
    data_to_send = bytes([0xFF, 0x00, 0x1A]) 
    
    packet = create_packet(command_id, data_to_send)
    print(f"Encoded Packet: {packet.hex().upper()}")