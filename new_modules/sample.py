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



// ResponseStatus rs;
// for (int address=2; address<6; address++){
//   Monitor.println();
//   Monitor.println("Asking Collar: #" + String(address));

//   rs = LoRa.sendFixedMessage(0, address, 0x0A, input);

//   unsigned long startTime = millis();
//   while (millis() - startTime < 3000) {
//     bool response = listenToLoRa();
//     if (response) {
//       Monitor.println("Got responce from: #" + String(address));
//       break;
//     }
//   }
//   Monitor.println("No responce from: #" + String(address));

// }
