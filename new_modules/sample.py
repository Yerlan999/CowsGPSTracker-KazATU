(BLUE+NIR)/(RED+SWIR3)


FireBeetle ESP-32
          Rx | Tx
Serial2 = 16 | 17
Serial1 =  9 | 10
Serial0 =  3 | 1

LoRa:STOP GPS
LoRa:START GPS
LoRa:TIME_UPDATE:1
LoRa:Test



float dummy_cyclic_coordinates[][2] = {
    {54.214176, 69.511151},
    {54.213195, 69.511303},
    {54.212246, 69.511400},
    {54.211856, 69.512168},
    {54.212011, 69.512357},
    {54.212327, 69.512357},
    {54.212580, 69.512519},
};


float dummy_cyclic_coordinates[][2] = {
    {54.213928, 69.510647},
    {54.212635, 69.511317},
    {54.211820, 69.512135},
    {54.212000, 69.512202},
    {54.212556, 69.512497},
    {54.213528, 69.513394},
    {54.214633, 69.514386},
};


latitude = dummy_coordinates[counter][0]; longitude = dummy_coordinates[counter][1];
latitude = dummy_coordinates[cowId][0]; longitude = dummy_coordinates[cowId][1];



+-----------------+-------------+----------+------------+-----------+
|   Target Column |   Precision |   Recall |   F1-Score |   Support |
+=================+=============+==========+============+===========+
|               0 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               1 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               2 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               3 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               4 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               5 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               6 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+
|               7 |           1 |        1 |          1 |     74088 |
+-----------------+-------------+----------+------------+-----------+



uint8_t cfg_rxm_disable_power_save[] = {
    0xB5, 0x62, // UBX Sync characters
    0x06, 0x11, // CFG-RXM message class and message id
    0x08, 0x00, // Payload length
    0x00, 0x00, // Flags (disable power save mode)
    0x01, 0x00, // Reserved
    0x13, 0x9E  // Checksum (CK_A and CK_B)
};

uint8_t cfg_rxm_disable_continuous[] = {
    0xB5, 0x62, // UBX Sync characters
    0x06, 0x11, // CFG-RXM message class and message id
    0x08, 0x00, // Payload length
    0x00, 0x00, // Flags (disable continuous mode)
    0x01, 0x00, // Reserved
    0x12, 0xA0  // Checksum (CK_A and CK_B)
};

uint8_t cfg_rxm_enable_power_save[] = {
    0xB5, 0x62, // UBX Sync characters
    0x06, 0x11, // CFG-RXM message class and message id
    0x08, 0x00, // Payload length
    0x01, 0x00, // Flags (enable power save mode)
    0x01, 0x00, // Reserved
    0x13, 0x9F  // Checksum (CK_A and CK_B)
};

uint8_t cfg_rxm_enable_continuous[] = {
    0xB5, 0x62, // UBX Sync characters
    0x06, 0x11, // CFG-RXM message class and message id
    0x08, 0x00, // Payload length
    0x02, 0x00, // Flags (enable continuous mode)
    0x01, 0x00, // Reserved
    0x14, 0xA1  // Checksum (CK_A and CK_B)
};