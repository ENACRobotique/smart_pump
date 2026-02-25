#!/usr/bin/env python3
import serial
import time
import sys

# PING
# ID: 01, Len: 02, Inst: 01 (PING), Checksum: FB
# Réponse attendue : 6 octets (Status packet sans erreur)
DATA_PING = b'\xFF\xFF\x01\x02\x01\xFB'

# READ COURANT (Float)
# ID: 01, Len: 04, Inst: 02 (READ), Addr: 20, Bytes: 04, Checksum: D4
# (0x01 + 0x04 + 0x02 + 0x20 + 0x04) = 0x2B -> Checksum = ~0x2B = 0xD4
# Réponse attendue : 10 octets (Header + ID + Len + Err + 4 bytes + Checksum)
DATA_READ = b'\xFF\xFF\x01\x04\x02\x20\x04\xD4'

# WRITE POMPE ON ('o')
# ID: 01, Len: 04, Inst: 03 (WRITE), Addr: 20, Value: 6F (lettre 'o' en hexa), Checksum: 68
# (0x01 + 0x04 + 0x03 + 0x20 + 0x6F) = 0x97 -> Checksum = ~0x97 = 0x68
# Réponse attendue : 6 octets (Status packet)
DATA_POMP_ON = b'\xFF\xFF\x01\x04\x03\x20\x6F\x68'

#Write pompe off ('p')
DATA_POMP_OFF = b'\xFF\xFF\x01\x04\x03\x20\x70\x67'

#Write Valve Ouvert ('v')
DATA_VALVE_OPEN = b'\xFF\xFF\x01\x04\x03\x20\x76\x61'

#Write Valve Fermer ('b')
DATA_VALVE_CLOSE = b'\xFF\xFF\x01\x04\x03\x20\x62\x75'

#Write Valva Use ('t')
DATA_VALVE_USE = b'\xFF\xFF\x01\x04\x03\x20\x74\x63'

DATAS = [
    (DATA_PING, 6, "PING"), 
    (DATA_READ, 8, "READ ADC"), 
    (DATA_POMP_ON, 6 , "PUMP ON 'o'"),
    (DATA_POMP_OFF, 6 , "PUMP OFF 'p'"),
    (DATA_VALVE_OPEN, 6 , "VALVE OUVERTE 'v'"),
    (DATA_VALVE_CLOSE, 6 , "VALVE FERMER 'b"),
    (DATA_VALVE_USE,-6, "VALVE USE  't'" )

]

if __name__ == "__main__":
    ser = serial.Serial(sys.argv[1], 115200, timeout=1)
    
    while True:
        for wdata, rlen, name in DATAS:
            print(f"--- Envoi de : {name} ---")
            
            ser.read_all() 
            ser.write(wdata)
            
            try:
                rdata = ser.read(rlen)
                
                if len(rdata) == rlen:
                    print(f"Réponse brute : {rdata.hex()}")
                else:
                    print(f"Timeout ou paquet incomplet. Reçu {len(rdata)} octets sur {rlen}")
            except Exception as e:
                print(f"Erreur de communication : {e}")

            time.sleep(1)
