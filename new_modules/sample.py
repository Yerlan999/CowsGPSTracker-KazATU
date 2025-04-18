(BLUE+NIR)/(RED+SWIR3)


FireBeetle ESP-32
          Rx | Tx
Serial2 = 16 | 17
Serial1 =  9 | 10
Serial0 =  3 | 1

// FireBeetle ESP32
LoRa_E32 LoRa(&Serial2, 13, 5, 2, UART_BPS_RATE_9600); //  Serial Pins, AUX, M0, M1
GPS.begin(9600, SERIAL_8N1, 26, 27); // RX, TX;

// ESP32
LoRa_E32 LoRa(&Serial2, 15, 22, 21, UART_BPS_RATE_9600); //  Serial Pins, AUX, M0, M1
GPS.begin(9600, SERIAL_8N1, 4, 2); // RX, TX;

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


How to apply UBX-CFG-PMS message in Ublox NEO-M8N GPS module?
UBX-CFG-RMX
UBX-CFG-PM2

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

# Receiver
LoRa.setMode(MODE_2_POWER_SAVING);
configuration.OPTION.wirelessWakeupTime = WAKE_UP_250;

# Sender
LoRa.setMode(MODE_1_WAKE_UP);
configuration.OPTION.wirelessWakeupTime = WAKE_UP_2000;


Содержание

1. Аналитическй обзор научно-технической информации по существующим проблемам и путей из решения в секторе цифрового управления пастбищеоборотом
1.1 Анализ текущего состояния и перспективы создания  интелектуальных систем кортроля выпаса КРС и контроля ресурса пастбища
1.2 Применение технологии дистанционного зондирования земли для оценки биомассы пастбища
1.3 Технические решения для рационального использования ресурсов пастбища
1.4 Выводы

2. Разработка модели(системы) по контролю состояния пастбища
2.1 Сбор и анализ снимков ДЗЗ
2.2 Разработка методики оценки ресурса пастбища с помощю даннных дистанционного зондирования земли
2.3 Разработка интеллектуальной платформы для оценки состояния пастбища
2.4 Выводы

3. Разработка конструкции и системы управления автоматическими воротами
3.1 Разработка конструкции автоматических ворот
3.2 Разработка системы управления автоматическими воротами
3.3 Разработка схемных решении по управлению
3.4 Выводы

4. Проведение экспериментальных исследовании и корректировка системы(комплекса)
4.1 Отладка по работе геопортала
4.2 Наладка технических средств и оценка работы автоматических ворот
4.3 Экспериментальные исследования по работе комплекса в целом. Оценка результатов
4.4 Выводы

Заключение


21, 19
22, 21


esp_sleep_wakeup_cause_t wakeup_reason;

wakeup_reason = esp_sleep_get_wakeup_cause();

if (ESP_SLEEP_WAKEUP_EXT0 == wakeup_reason) {
    Monitor.println("Waked up from external GPIO!");

    gpio_hold_dis(GPIO_NUM_22);
    gpio_hold_dis(GPIO_NUM_21);

    gpio_deep_sleep_hold_dis();

    LoRa.setMode(MODE_0_NORMAL);

    delay(1000);

    ResponseStatus responce = writeToLoRa("We have waked up from message, but we can't read It!");
}else{
    LoRa.setMode(MODE_2_POWER_SAVING);

    delay(1000);
    Monitor.println();
    Monitor.println("Start sleep!");
    delay(100);

    if (ESP_OK == gpio_hold_en(GPIO_NUM_22)){
        Monitor.println("HOLD 22");
    }else{
        Monitor.println("NO HOLD 22");
    }
    if (ESP_OK == gpio_hold_en(GPIO_NUM_21)){
            Monitor.println("HOLD 21");
        }else{
            Monitor.println("NO HOLD 21");
        }

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,LOW);

    gpio_deep_sleep_hold_en();
    //Go to sleep now
    Monitor.println("Going to sleep now");
    esp_deep_sleep_start();

    delay(1);
}






#include <SoftwareSerial.h>

// Define the RX and TX pins for the SoftwareSerial communication with the GPS module
#define GPS_RX_PIN 2
#define GPS_TX_PIN 3

// Create a SoftwareSerial object to communicate with the GPS module
SoftwareSerial GPS(GPS_RX_PIN, GPS_TX_PIN);

// Define the payload for Balanced Power saving mode
const uint8_t balancedPowerSavingPayload[] = {
  0x00, // Version
  0x01, // Reserve1
  0x10, // Reserve2
  0x10, // Reserve3
  0x05, // Power Setup: Balanced Power Saving mode
  0x05, // Period: Not used in Balanced mode (set to 0)
  0x00, // OnTime: Not used in Balanced mode (set to 0)
  0x00, // MinTime: Not used in Balanced mode (set to 0)
};

void setup() {
  // Initialize serial communication with the computer
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Initialize serial communication with the GPS module
  GPS.begin(9600);

  // Configure the power management settings of the GPS module
  configurePowerManagement();
}

void loop() {
  // Main loop code
}

void configurePowerManagement() {
  // Send the UBX-CFG-PMS message to the GPS module for Balanced Power saving mode
  sendUBXMessage(0x06, 0x86, balancedPowerSavingPayload, sizeof(balancedPowerSavingPayload));
}

void sendUBXMessage(uint8_t msgClass, uint8_t msgId, const uint8_t* payload, uint16_t length) {
  // Send UBX header
  GPS.write(0xB5);
  GPS.write(0x62);

  // Send message class and ID
  GPS.write(msgClass);
  GPS.write(msgId);

  // Send payload length
  GPS.write(length & 0xFF);
  GPS.write(length >> 8);

  // Send payload
  for (int i = 0; i < length; i++) {
    GPS.write(payload[i]);
  }

  // Calculate and send checksum
  uint8_t checksumA = 0, checksumB = 0;
  for (int i = 0; i < length; i++) {
    checksumA += payload[i];
    checksumB += checksumA;
  }
  GPS.write(checksumA);
  GPS.write(checksumB);
}




configuration.OPTION.fec = FEC_1_ON;
configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
configuration.OPTION.transmissionPower = POWER_20;

configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
configuration.SPED.uartBaudRate = UART_BPS_9600;
configuration.SPED.uartParity = MODE_00_8N1;




$GNGLL,5110.15984,N,07122.50811,E,112726.00,A,A*79
$GNRMC,112727.00,A,5110.15985,N,07122.50834,E,0.159,,240424,,,A*6E
$GNVTG,,T,,M,0.159,N,0.295,K,A*3E
$GNGGA,112727.00,5110.15985,N,07122.50834,E,1,06,3.12,358.0,M,-33.2,M,,*6C
$GNGSA,A,3,15,05,29,13,20,,,,,,,,4.50,3.12,3.25*13
$GNGSA,A,3,69,,,,,,,,,,,,4.50,3.12,3.25*16
$GPGSV,3,1,10,05,52,061,29,07,02,023,,13,31,100,19,15,34,143,22*7C
$GPGSV,3,2,10,16,19,318,,18,58,283,,20,21,058,27,23,23,223,*7B
$GPGSV,3,3,10,26,26,281,,29,63,166,24*7A
$GLGSV,3,1,09,68,34,037,,69,61,112,34,70,17,183,,75,18,272,*69
$GLGSV,3,2,09,76,19,331,,77,01,010,,83,09,107,,84,67,068,*62
$GLGSV,3,3,09,85,42,309,*5D

$GNGLL,5110.15985,N,07122.50834,E,112727.00,A,A*7E
$GNRMC,112728.00,A,5110.15983,N,07122.50842,E,0.169,,240424,,,A*65
$GNVTG,,T,,M,0.169,N,0.314,K,A*35
$GNGGA,112728.00,5110.15983,N,07122.50842,E,1,06,3.12,358.0,M,-33.2,M,,*64
$GNGSA,A,3,15,05,29,13,20,,,,,,,,4.50,3.12,3.25*13
$GNGSA,A,3,69,,,,,,,,,,,,4.50,3.12,3.25*16
$GPGSV,3,1,10,05,52,061,28,07,02,023,,13,31,100,24,15,34,143,23*72
$GPGSV,3,2,10,16,19,318,,18,58,283,,20,21,058,27,23,23,223,*7B
$GPGSV,3,3,10,26,26,281,,29,63,166,25*7B
$GLGSV,3,1,09,68,34,037,,69,61,112,34,70,17,183,,75,18,272,*69
$GLGSV,3,2,09,76,19,331,,77,01,010,,83,09,107,,84,67,068,*62
$GLGSV,3,3,09,85,42,309,*5D



B5 62 06 86 08 00 00 00 00 00 00 00 00 00 94 5A // Full power
B5 62 06 86 08 00 00 01 00 00 00 00 00 00 95 61 // Balanced
B5 62 06 86 08 00 00 03 00 00 00 00 00 00 97 6F // Agressive 1Hz


B5 62 06 57 08 00 01 00 00 00 50 4B 43 42 86 46 // Software Backup
B5 62 06 57 08 00 01 00 00 00 50 4F 54 53 AC 85 // GNSS stoppen
B5 62 06 57 08 00 01 00 00 00 20 4E 55 52 7B C3 // GNSS running

B5 62 06 11 02 00 08 00 21 91 // Continuous mode
B5 62 06 11 02 00 08 01 22 92 // Power save mode


0xB5 0x62 0x06 0x86 0x08 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x94 0x5A // Full power
0xB5 0x62 0x06 0x86 0x08 0x00 0x00 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x95 0x61 // Balanced
0xB5 0x62 0x06 0x86 0x08 0x00 0x00 0x03 0x00 0x00 0x00 0x00 0x00 0x00 0x97 0x6F // Agressive 1Hz

0xB5 0x62 0x06 0x57 0x08 0x00 0x01 0x00 0x00 0x00 0x50 0x4B 0x43 0x42 0x86 0x46 // Software Backup
0xB5 0x62 0x06 0x57 0x08 0x00 0x01 0x00 0x00 0x00 0x50 0x4F 0x54 0x53 0xAC 0x85 // GNSS stoppen
0xB5 0x62 0x06 0x57 0x08 0x00 0x01 0x00 0x00 0x00 0x20 0x4E 0x55 0x52 0x7B 0xC3 // GNSS running

0xB5 0x62 0x06 0x11 0x02 0x00 0x08 0x00 0x21 0x91 // Continuous mode
0xB5 0x62 0x06 0x11 0x02 0x00 0x08 0x01 0x22 0x92 // Power save mode



// GNSS stoppen
uint8_t gnssStopCmd[] = {   0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x00, 0x00, 0x00, 0x53, 0x54, 0x4F, 0x50, 0xAB, 0x26};
// GNSS running
uint8_t gnssRunningCmd[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x00, 0x00, 0x00, 0x52, 0x55, 0x4E, 0x20, 0x7A, 0xF3};

// Full power
uint8_t fullPowerCmd[] = {0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x6C};
// Balanced
uint8_t balancedCmd[] = { 0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x95, 0xCD};
// Aggressive 1Hz
uint8_t aggressiveCmd[] ={0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0xDB};

// Software Backup
uint8_t softwareBackupCmd[] = {0xB5, 0x62, 0x06, 0x57, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x4B, 0x43, 0x42, 0x86, 0x46};

// Continuous mode
uint8_t continuousModeCmd[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x00, 0x19, 0x68};
// Power save mode
uint8_t powerSaveModeCmd[] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x01, 0x1A, 0x69};



void GPS_SendConfig(const uint8_t *Progmem_ptr) {
  uint8_t byteread;

  // Calculate the array size
  size_t arraysize = sizeof(Progmem_ptr) / sizeof(uint8_t);

  Serial.print(F("GPSSend  "));

  for (size_t index = 0; index < arraysize; index++) {
    byteread = pgm_read_byte_near(Progmem_ptr++);
    if (byteread < 0x10) {
      Serial.print(F("0"));
    }
    Serial.print(byteread, HEX);
    Serial.print(F(" "));
  }

  Serial.println();
  Progmem_ptr = Progmem_ptr - arraysize;  // Set Progmem_ptr back to start

  for (size_t index = 0; index < arraysize; index++) {
    byteread = pgm_read_byte_near(Progmem_ptr++);
    GPS.write(byteread);
  }
  delay(100);
}


ESP32 ||  XH2.45  ||  GPS
Pin2  ->  RED    -> YELLOW (RX)
Pin4  ->  BLACK  -> GREEN (TX)
GND   ->  YELLOW -> BLACK
VCC   ->  WHITE  -> RED


PM - Power Management
PM2 - Extended Power Management
PMS - Power Management Setup
RXM - Reveiver Manager


START = 67
NUMBER = 30

for i, j in enumerate(range(NUMBER), start=START):
    print(i)




x = ["№4", "№5", "№6", "№7"]
y = [2.2, 1.9, 0.9, 0.6]
correlation_dict = dict()

# Plot with markers
plt.plot(x, y, marker='o', color='blue', label='Значение')

# Customize axes and title
plt.xlabel('Загоны (№)')
plt.ylabel('Урожайность (т/га)')
plt.title(f'Урожайность загонов на дату {start_date}')

# Add a legend
plt.legend()

# Show the plot
plt.show()



Arithmetic mean == Среднее арифметическое
Harmonic mean == Среднее гармоническое
Geometric mean == Среднее геометрическое
Median == Медиана
Standard deviation == Стандартное отклонение
Kurtosis (Excess kurtosis) == Коэффициент эксцесса
Skewness == Коэффициент ассиметрии
Interquartile range (IQR) == Межквартильный размах



# English Version
def show_hist_and_scatter(zagon, all_pasture_OR_each_paddock, paddock_id, header):
    i = paddock_id

    fig, axs = plt.subplots(1, 2, figsize=(16, 6))  # Create side-by-side subplots

    # Histogram plot (left subplot)
    ax_hist = axs[0]
    n, bins, patches = ax_hist.hist(
        zagon.compressed(),  # Use .compressed() to handle masked arrays
        bins=30,
        color=colors[i],
        alpha=0.5,
        edgecolor='black',
        label="Histogram"
    )

    # Calculate statistics
    mean = zagon.mean()
    median = ma.median(zagon)
    std_dev = zagon.std()
    flattened_matrix = zagon.compressed()
    skewness_value = skew(flattened_matrix)
    Q1 = np.percentile(flattened_matrix, 25)
    Q3 = np.percentile(flattened_matrix, 75)
    IQR = Q3 - Q1
    kurtosis_value = kurtosis(flattened_matrix)

    # Add vertical lines
    ax_hist.axvline(zagon.max(), color='r', linestyle=':', linewidth=2, label=f"Max: {zagon.max()}")
    ax_hist.axvline(zagon.min(), color='b', linestyle=':', linewidth=2, label=f"Min: {zagon.min()}")
    ax_hist.axvline(mean, color='b', linestyle='dashed', linewidth=2, label=f"Arithmetic mean: {mean}")
    ax_hist.axvline(median, color='r', linestyle='dashed', linewidth=2, label=f"Median: {median}")

    # Mask negative or zero values for harmonic and geometric means
    if not (zagon <= 0).sum():
        harmonic_mean = hmean(flattened_matrix)
        geometric_mean = gmean(flattened_matrix)
        ax_hist.axvline(harmonic_mean, color='g', linestyle='dashed', linewidth=2, label=f"Harmonic mean: {harmonic_mean}")
        ax_hist.axvline(geometric_mean, color='y', linestyle='dashed', linewidth=2, label=f"Geometric mean: {geometric_mean}")

    # Bell curve
    x = np.linspace(bins[0], bins[-1], 500)
    bell_curve = norm.pdf(x, loc=mean, scale=std_dev)
    bell_curve_scaled = bell_curve * n.max() / bell_curve.max()
    ax_hist.plot(x, bell_curve_scaled, color='m', linewidth=2, label="Normal distribution")

    # Annotate histogram with additional statistics
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Standard deviation: {std_dev}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Skewness: {skewness_value}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Interquartile range (IQR): {IQR}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Kurtosis (Excess kurtosis): {kurtosis_value}', alpha=0)

    # Finalize histogram
    ax_hist.legend(title=f'Sum: {round(zagon.sum(), precision)}')

    if all_pasture_OR_each_paddock:
        ax_hist.set_title(f'{header} || Pasture {general_info}')
    else:
        ax_hist.set_title(f'{header} || Paddock-{i+1} {general_info}')

    ax_hist.set_xlabel('Value')
    ax_hist.set_ylabel('Frequency')

    # Scatter plot (right subplot)
    ax_scatter = axs[1]
    x = np.arange(zagon.size)
    y = zagon.flatten()
    unmasked_values = zagon.compressed()

    mean_value = np.ma.mean(zagon)
    median_value = np.ma.median(zagon)
    if not (zagon <= 0).sum():
        geometric_mean = gmean(unmasked_values)
        harmonic_mean = hmean(unmasked_values)

    ax_scatter.scatter(x, y, s=100, label="Values", color=colors[i])
    ax_scatter.axhline(y=mean_value, color='b', linestyle='--', label=f'Arithmetic mean: {mean_value}')
    ax_scatter.axhline(y=median_value, color='r', linestyle='--', label=f'Median: {median_value}')
    if not (zagon <= 0).sum():
        ax_scatter.axhline(y=geometric_mean, color='y', linestyle='--', label=f'Geometric mean: {geometric_mean}')
        ax_scatter.axhline(y=harmonic_mean, color='g', linestyle='--', label=f'Harmonic mean: {harmonic_mean}')

    if all_pasture_OR_each_paddock:
        ax_scatter.set_title(f'{header} || Pasture {general_info}')
    else:
        ax_scatter.set_title(f'{header} || Paddock-{i+1} {general_info}')

    ax_scatter.set_xlabel('Index')
    ax_scatter.set_ylabel('Value')
    ax_scatter.legend()

    plt.tight_layout()
    plt.show()


summary_df = None
def show_pasture_index(test_meet, lower_bound, upper_bound, show_hists=True, save_excel=False, show_index=True, show_table=True):
    global summary_df
    precision = 4
    header = None

    if show_index:
        fig, ax = plt.subplots(figsize=(12, 12))
        for zagon in range(len(pasture_df)-1):

            ax.plot(pasture_edges[zagon].exterior.xy[1], pasture_edges[zagon].exterior.xy[0])

        header = input_text
        # print(f"Max: {round(float(test_meet.max()),precision)} || Min: {round(float(test_meet.min()),precision)} || Arithmetic mean: {round(float(test_meet.mean()),precision)} || Sum: {round(float(test_meet.sum()),precision)}")
        ep.plot_bands(test_meet, title=f"{header} {general_info}", ax=ax, cmap="seismic", cols=1, vmin=lower_bound, vmax=upper_bound, figsize=(10, 14))
        plt.show()

    test_index_masked_array = []
    for i, mask in enumerate(masks):
        mx = ma.masked_array(test_meet, mask=mask.reshape(aoi_height, aoi_width))
        test_index_masked_array.append(mx)


    weather_parameters = history_df[history_df["time"]==date_chosen]

    weather_parameters = weather_parameters.assign(
        sunZenithAngles=SZA,
        sunAzimuthAngles=SAA,
        viewZenithMean=VZM,
        viewAzimuthMean=VAM)

#     weather_parameters = weather_parameters.drop(columns=["time"], inplace=False)
#     weighted_vector = (weather_parameters * pd.Series(weights)).sum(axis=1)
#     print(weighted_vector)

#     NWC = (float(weighted_vector.iloc[0]) - 393.2550463180542) / (590.1110732021332 - 393.2550463180542)

    summary_data = []
    for i, zagon in enumerate(test_index_masked_array):
#         resource = round(8.59 * ma.median(zagon) + 0.20 * NWC + -7.04, precision)

        geometric_mean = round(float(gmean(zagon.reshape(aoi_width * aoi_height))),precision)
        harmonic_mean = round(float(hmean(zagon.reshape(aoi_width * aoi_height))),precision)

        std_dev = zagon.std()
        flattened_matrix = zagon.compressed()
        skewness_value = skew(flattened_matrix)
        Q1 = np.percentile(flattened_matrix, 25); Q3 = np.percentile(flattened_matrix, 75)
        IQR = Q3 - Q1
        kurtosis_value = kurtosis(flattened_matrix)

        temp_resource = 8.87 * ma.median(zagon) + 0.03 * alter_weather_param_df["temp"].iloc[0] -7.91 # (Temperature)
        press_resource = 9.12 * ma.median(zagon) -0.01 * alter_weather_param_df["pressure"].iloc[0] + 2.18 # (Pressure)
        humid_resource = 8.49 * ma.median(zagon) -0.00 * alter_weather_param_df["humidity"].iloc[0] -6.66 # (Moisture)
        angle_resource = 8.95 * ma.median(zagon) -0.03 * (SZA + VZM) -6.06 # (SZA + VZM = RZA)

        resource = round(np.array([temp_resource, press_resource, humid_resource, angle_resource]).mean(), precision)

        summary_data.append([f"№{i+1}", round(zagon.sum(),precision), round(zagon.mean(),precision), round(ma.median(zagon),precision), geometric_mean, harmonic_mean, round(std_dev,precision), round(skewness_value,precision), round(IQR,precision), round(kurtosis_value,precision), round(zagon.max(),precision), round(zagon.min(),precision), round(ma.count(zagon)*(10**2)*0.0001,precision), resource])


    styles = [
        {'selector': '',
         'props': [('border', '2px solid #000'), ('border-collapse', 'collapse')]},
        {'selector': 'th',
         'props': [('border', '2px solid #000')]},
        {'selector': 'td',
         'props': [('border', '1px solid #000'), ('padding', '5px')]}
    ]

    summary_df = pd.DataFrame(data = summary_data, columns=["Paddock", "Sum", "Arithmetic mean", "Median", 'Geometric mean', 'Harmonic mean', "Standard deviation", "Skewness", "Interquartile range (IQR)", "Kurtosis (Excess kurtosis)", "Max", "Min", "Area (ha)", "Resource (t/ha)"])

    std_dev = test_meet.std()
    flattened_matrix = test_meet.compressed()
    skewness_value = skew(flattened_matrix)
    Q1 = np.percentile(flattened_matrix, 25); Q3 = np.percentile(flattened_matrix, 75)
    IQR = Q3 - Q1
    kurtosis_value = kurtosis(flattened_matrix)

    sum_row = pd.DataFrame({'Paddock': ["Pasture"],
                            'Sum': [round(float(summary_df['Sum'].sum()),precision)],
                            'Arithmetic mean': [round(float(test_meet.mean()),precision)],
                            'Median': [round(float(ma.median(test_meet)),precision)],

                            'Geometric mean': [round(float(gmean(test_meet.reshape(aoi_width * aoi_height))),precision)],
                            'Harmonic mean': [round(float(hmean(test_meet.reshape(aoi_width * aoi_height))),precision)],

                            'Standard deviation': [round(float(std_dev),precision)],
                            'Skewness': [round(float(skewness_value),precision)],
                            'Interquartile range (IQR)': [round(float(IQR),precision)],
                            'Kurtosis (Excess kurtosis)': [round(float(kurtosis_value),precision)],

                            'Max': [round(float(summary_df['Max'].max()),precision)],
                            'Min': [round(float(summary_df['Min'].min()),precision)],
                            'Area (ha)': [summary_df['Area (ha)'].sum()],
                            'Resource (t/ha)': [summary_df['Resource (t/ha)'].sum()]}, index=[len(summary_df.index)])

    summary_df = pd.concat([summary_df, sum_row])
    summary_df = summary_df.round(precision)

    if save_excel:
        summary_df.to_excel(f"Summary_{date_chosen}_{data_collection.processing_level}.xlsx", index=None)
    styled_df = summary_df.style.set_table_styles(styles)
    styled_df.hide(axis="index")

    if show_table:
        display(styled_df)

    if show_hists:
        show_hist_and_scatter(test_meet, True, 27, header)
        # Loop through each dataset in test_index_masked_array
#         for i, zagon in enumerate(test_index_masked_array):
#             show_hist_and_scatter(zagon, False, i, header)


# Russian Version
def show_hist_and_scatter(zagon, all_pasture_OR_each_paddock, paddock_id, header):
    i = paddock_id

    fig, axs = plt.subplots(1, 2, figsize=(16, 6))  # Create side-by-side subplots

    # Histogram plot (left subplot)
    ax_hist = axs[0]
    n, bins, patches = ax_hist.hist(
        zagon.compressed(),  # Use .compressed() to handle masked arrays
        bins=30,
        color=colors[i],
        alpha=0.5,
        edgecolor='black',
        label="Гистограмма"
    )

    # Calculate statistics
    mean = zagon.mean()
    median = ma.median(zagon)
    std_dev = zagon.std()
    flattened_matrix = zagon.compressed()
    skewness_value = skew(flattened_matrix)
    Q1 = np.percentile(flattened_matrix, 25)
    Q3 = np.percentile(flattened_matrix, 75)
    IQR = Q3 - Q1
    kurtosis_value = kurtosis(flattened_matrix)

    # Add vertical lines
    ax_hist.axvline(zagon.max(), color='r', linestyle=':', linewidth=2, label=f"Макс.: {zagon.max()}")
    ax_hist.axvline(zagon.min(), color='b', linestyle=':', linewidth=2, label=f"Мин.: {zagon.min()}")
    ax_hist.axvline(mean, color='b', linestyle='dashed', linewidth=2, label=f"Средняя: {mean}")
    ax_hist.axvline(median, color='r', linestyle='dashed', linewidth=2, label=f"Медианная: {median}")

    # Mask negative or zero values for harmonic and geometric means
    if not (zagon <= 0).sum():
        harmonic_mean = hmean(flattened_matrix)
        geometric_mean = gmean(flattened_matrix)
        ax_hist.axvline(harmonic_mean, color='g', linestyle='dashed', linewidth=2, label=f"Гармоническая: {harmonic_mean}")
        ax_hist.axvline(geometric_mean, color='y', linestyle='dashed', linewidth=2, label=f"Геометрическая: {geometric_mean}")

    # Bell curve
    x = np.linspace(bins[0], bins[-1], 500)
    bell_curve = norm.pdf(x, loc=mean, scale=std_dev)
    bell_curve_scaled = bell_curve * n.max() / bell_curve.max()
    ax_hist.plot(x, bell_curve_scaled, color='m', linewidth=2, label="Нормальное распределение")

    # Annotate histogram with additional statistics
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Стандарт. отклонение: {std_dev}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Коэффициент асимметрии: {skewness_value}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Межквартильный размах: {IQR}', alpha=0)
    ax_hist.axvline(mean, color='white', linestyle='--', label=f'Величина эксцесса: {kurtosis_value}', alpha=0)

    # Finalize histogram
    ax_hist.legend(title=f'Сумма: {round(zagon.sum(), precision)}')

    if all_pasture_OR_each_paddock:
        ax_hist.set_title(f'{header} || Пастбище {general_info}')
    else:
        ax_hist.set_title(f'{header} || Загон-{i+1} {general_info}')

    ax_hist.set_xlabel('Значение')
    ax_hist.set_ylabel('Частота')

    # Scatter plot (right subplot)
    ax_scatter = axs[1]
    x = np.arange(zagon.size)
    y = zagon.flatten()
    unmasked_values = zagon.compressed()

    mean_value = np.ma.mean(zagon)
    median_value = np.ma.median(zagon)
    if not (zagon <= 0).sum():
        geometric_mean = gmean(unmasked_values)
        harmonic_mean = hmean(unmasked_values)

    ax_scatter.scatter(x, y, s=100, label="Значения", color=colors[i])
    ax_scatter.axhline(y=mean_value, color='b', linestyle='--', label=f'Средняя: {mean_value}')
    ax_scatter.axhline(y=median_value, color='r', linestyle='--', label=f'Медианная: {median_value}')
    if not (zagon <= 0).sum():
        ax_scatter.axhline(y=geometric_mean, color='y', linestyle='--', label=f'Геометрическая: {geometric_mean}')
        ax_scatter.axhline(y=harmonic_mean, color='g', linestyle='--', label=f'Гармоническая: {harmonic_mean}')

    if all_pasture_OR_each_paddock:
        ax_scatter.set_title(f'{header} || Пастбище {general_info}')
    else:
        ax_scatter.set_title(f'{header} || Загон-{i+1} {general_info}')

    ax_scatter.set_xlabel('Индекс')
    ax_scatter.set_ylabel('Значение')
    ax_scatter.legend()

    plt.tight_layout()
    plt.show()


summary_df = None
def show_pasture_index(test_meet, lower_bound, upper_bound, show_hists=True, save_excel=False, show_index=True, show_table=True):
    global summary_df
    precision = 4
    header = None

    if show_index:
        fig, ax = plt.subplots(figsize=(12, 12))
        for zagon in range(len(pasture_df)-1):

            ax.plot(pasture_edges[zagon].exterior.xy[1], pasture_edges[zagon].exterior.xy[0])

        header = input_text
        # print(f"Макс: {round(float(test_meet.max()),precision)} || Мин: {round(float(test_meet.min()),precision)} || Сред: {round(float(test_meet.mean()),precision)} || Сумм: {round(float(test_meet.sum()),precision)}")
        ep.plot_bands(test_meet, title=f"{header} {general_info}", ax=ax, cmap="seismic", cols=1, vmin=lower_bound, vmax=upper_bound, figsize=(10, 14))
        plt.show()

    test_index_masked_array = []
    for i, mask in enumerate(masks):
        mx = ma.masked_array(test_meet, mask=mask.reshape(aoi_height, aoi_width))
        test_index_masked_array.append(mx)


    weather_parameters = history_df[history_df["time"]==date_chosen]

    weather_parameters = weather_parameters.assign(
        sunZenithAngles=SZA,
        sunAzimuthAngles=SAA,
        viewZenithMean=VZM,
        viewAzimuthMean=VAM)

#     weather_parameters = weather_parameters.drop(columns=["time"], inplace=False)
#     weighted_vector = (weather_parameters * pd.Series(weights)).sum(axis=1)
#     print(weighted_vector)

#     NWC = (float(weighted_vector.iloc[0]) - 393.2550463180542) / (590.1110732021332 - 393.2550463180542)

    summary_data = []
    for i, zagon in enumerate(test_index_masked_array):
#         resource = round(8.59 * ma.median(zagon) + 0.20 * NWC + -7.04, precision)

        geometric_mean = round(float(gmean(zagon.reshape(aoi_width * aoi_height))),precision)
        harmonic_mean = round(float(hmean(zagon.reshape(aoi_width * aoi_height))),precision)

        std_dev = zagon.std()
        flattened_matrix = zagon.compressed()
        skewness_value = skew(flattened_matrix)
        Q1 = np.percentile(flattened_matrix, 25); Q3 = np.percentile(flattened_matrix, 75)
        IQR = Q3 - Q1
        kurtosis_value = kurtosis(flattened_matrix)

        temp_resource = 8.87 * ma.median(zagon) + 0.03 * alter_weather_param_df["temp"].iloc[0] -7.91 # (Temperature)
        press_resource = 9.12 * ma.median(zagon) -0.01 * alter_weather_param_df["pressure"].iloc[0] + 2.18 # (Pressure)
        humid_resource = 8.49 * ma.median(zagon) -0.00 * alter_weather_param_df["humidity"].iloc[0] -6.66 # (Moisture)
        angle_resource = 8.95 * ma.median(zagon) -0.03 * (SZA + VZM) -6.06 # (SZA + VZM = RZA)

        resource = round(np.array([temp_resource, press_resource, humid_resource, angle_resource]).mean(), precision)

        summary_data.append([f"№{i+1}", round(zagon.sum(),precision), round(zagon.mean(),precision), round(ma.median(zagon),precision), geometric_mean, harmonic_mean, round(std_dev,precision), round(skewness_value,precision), round(IQR,precision), round(kurtosis_value,precision), round(zagon.max(),precision), round(zagon.min(),precision), round(ma.count(zagon)*(10**2)*0.0001,precision), resource])


    styles = [
        {'selector': '',
         'props': [('border', '2px solid #000'), ('border-collapse', 'collapse')]},
        {'selector': 'th',
         'props': [('border', '2px solid #000')]},
        {'selector': 'td',
         'props': [('border', '1px solid #000'), ('padding', '5px')]}
    ]

    summary_df = pd.DataFrame(data = summary_data, columns=["Загон", "Сумма", "Cреднаяя", "Медианная", 'Геомет.сред.', 'Гармон.сред.', "Станд. откл.", "Коэф. асимм.", "Межкв. размах", "Велич. эксц.", "Макс", "Мин", "Площадь (га)", "Ресурс (т/га)"])

    std_dev = test_meet.std()
    flattened_matrix = test_meet.compressed()
    skewness_value = skew(flattened_matrix)
    Q1 = np.percentile(flattened_matrix, 25); Q3 = np.percentile(flattened_matrix, 75)
    IQR = Q3 - Q1
    kurtosis_value = kurtosis(flattened_matrix)

    sum_row = pd.DataFrame({'Загон': ["Пастбище"],
                            'Сумма': [round(float(summary_df['Сумма'].sum()),precision)],
                            'Cреднаяя': [round(float(test_meet.mean()),precision)],
                            'Медианная': [round(float(ma.median(test_meet)),precision)],

                            'Геомет.сред.': [round(float(gmean(test_meet.reshape(aoi_width * aoi_height))),precision)],
                            'Гармон.сред.': [round(float(hmean(test_meet.reshape(aoi_width * aoi_height))),precision)],

                            'Станд. откл.': [round(float(std_dev),precision)],
                            'Коэф. асимм.': [round(float(skewness_value),precision)],
                            'Межкв. размах': [round(float(IQR),precision)],
                            'Велич. эксц.': [round(float(kurtosis_value),precision)],

                            'Макс': [round(float(summary_df['Макс'].max()),precision)],
                            'Мин': [round(float(summary_df['Мин'].min()),precision)],
                            'Площадь (га)': [summary_df['Площадь (га)'].sum()],
                            'Ресурс (т/га)': [summary_df['Ресурс (т/га)'].sum()]}, index=[len(summary_df.index)])

    summary_df = pd.concat([summary_df, sum_row])
    summary_df = summary_df.round(precision)

    if save_excel:
        summary_df.to_excel(f"Summary_{date_chosen}_{data_collection.processing_level}.xlsx", index=None)
    styled_df = summary_df.style.set_table_styles(styles)
    styled_df.hide(axis="index")

    if show_table:
        display(styled_df)

    if show_hists:
        show_hist_and_scatter(test_meet, True, 27, header)
        # Loop through each dataset in test_index_masked_array
#         for i, zagon in enumerate(test_index_masked_array):
#             show_hist_and_scatter(zagon, False, i, header)







# English Version
filtered_df = summary_df[summary_df['Paddock'].isin(["№4", "№5", "№6", "№7"])]

# Select a range of columns to plot (e.g., all columns except 'Paddock')
columns_to_plot = filtered_df.columns[1:-2]  # Skip the first column ('Paddock')
correlation_list = []

# Plot each column dynamically
plt.figure(figsize=(10, 6))
for column in columns_to_plot:

    correlation = np.corrcoef(filtered_df[column], y)[0, 1]
    correlation_list.append(correlation)

    if column == "Sum":
        plt.plot(filtered_df['Paddock'], scale_to_unit_range_np(filtered_df[column]), marker='o', label=f"{column} (*k = {correlation:.4f})")
    else:
        plt.plot(filtered_df['Paddock'], filtered_df[column], marker='o', label=f"{column} (k = {correlation:.4f})")

correlation_dict[input_text] = correlation_list

# Add labels, title, and legend
plt.xlabel('Paddocks (№)')
plt.ylabel('Value')
plt.title(f'Statistical indicators of channel {input_text} for date {start_date}')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()


# Russian Version
filtered_df = summary_df[summary_df['Загон'].isin(["№4", "№5", "№6", "№7"])]

# Select a range of columns to plot (e.g., all columns except 'Загон')
columns_to_plot = filtered_df.columns[1:-2]  # Skip the first column ('Загон')
correlation_list = []

# Plot each column dynamically
plt.figure(figsize=(10, 6))
for column in columns_to_plot:

    correlation = np.corrcoef(filtered_df[column], y)[0, 1]
    correlation_list.append(correlation)

    if column == "Сумма":
        plt.plot(filtered_df['Загон'], scale_to_unit_range_np(filtered_df[column]), marker='o', label=f"{column} (*k = {correlation:.4f})")
    else:
        plt.plot(filtered_df['Загон'], filtered_df[column], marker='o', label=f"{column} (k = {correlation:.4f})")

correlation_dict[input_text] = correlation_list

# Add labels, title, and legend
plt.xlabel('Загоны (№)')
plt.ylabel('Значение')
plt.title(f'Статистические показатели канала {input_text} на дату {start_date}')
plt.legend()
plt.grid(True)

# Show the plot
plt.show()
