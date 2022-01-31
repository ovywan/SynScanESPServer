// SynScan binary message
// User Position, Velocity & Time II (D1h)
struct BinaryMsg {
  uint16_t weekNo;
  uint32_t timeOfWeek;
  uint32_t date;
  uint32_t time;
  int32_t latitude;
  int32_t longitude;
  int16_t  altitude;
  uint16_t heading;
  uint16_t speed;
  uint8_t  fixIndicator;
  uint8_t  qualityOfFix;
  uint8_t  numberOfSv;
  uint8_t  numberOfSvInFix;
  uint8_t  gdop;
  uint8_t  pdop;
  uint8_t  hdop;
  uint8_t  vdop;
  uint8_t  tdop;
} __attribute__((packed));

uint8_t buf1[bufferSize], buf2[bufferSize];
uint8_t i1 = 0, i2 = 0;

uint32_t GPS_initial_timer, WiFi_client_timer;
bool GPS_initialized = false, GPS_terminated = false, WiFi_initialized = false, WiFi_server_active = false, CurrentMode = false;

// GPS class
SoftwareSerial GPS_Serial(2, -1);
Adafruit_GPS gps(&GPS_Serial);

// SynScan connexion
#define controller Serial

// Global State
bool sendBinaryMsg = false;

// Synscan command buffer
#define BUFF_SIZE 15
char synscanBuff[BUFF_SIZE] = {0};
uint8_t synscanBuffOffset = 0;

uint8_t uint2bcd(uint8_t ival) {
  return ((ival / 10) << 4) | (ival % 10);
}

// Compute binary message checksum
static char computeChecksum(char* buf, uint16_t len) {
  char chksum = 0;

  for (uint16_t i = 0; i < len; i++)
  {
    chksum ^= buf[i];
  }

  return chksum;
}

static void synscanSendMsg(char *msg, uint16_t len) {
  for (uint16_t i = 0; i < len; i++)
  {
    controller.write(msg[i]);
  }
}

static void synscanEncode(char c) {
  // Add byte to buffer
  if (synscanBuffOffset < BUFF_SIZE - 1)
  {
    synscanBuff[synscanBuffOffset] = c;
    synscanBuffOffset++;
  }

  if (c == '\n')
  {
    // End of command
    if (strncmp(synscanBuff, "%%\xf1\x13\x00\xe2\r\n", synscanBuffOffset) == 0)
    {
      // No output
      sendBinaryMsg = false;

      // Send ack
      char msg[7] = {'%', '%', '\x06', '\x13', '\x15', '\r', '\n'};
      synscanSendMsg(msg, 7);
    }
    else if (strncmp(synscanBuff, "%%\xf1\x13\x03\xe1\r\n", synscanBuffOffset) == 0)
    {
      // Binary output
      sendBinaryMsg = true;

      // Send ack
      char msg[7] = {'%', '%', '\x06', '\x13', '\x15', '\r', '\n'};
      synscanSendMsg(msg, 7);
    }

    // Clean buff
    synscanBuffOffset = 0;
  }
}

static void synscanSendBinMsg(BinaryMsg *binMsg) {
  uint16_t size = 4 + sizeof(BinaryMsg);
  char msg[size];

  msg[0] = '%';
  msg[1] = '%';
  msg[2] = '\xf2';
  msg[3] = '\xd1';

  memcpy(msg + 4, binMsg, sizeof(BinaryMsg));

  for (uint16_t i = 0; i < size; i++)
  {
    controller.write(msg[i]);
  }

  // Checksum
  controller.write(computeChecksum(msg, size));

  // End
  controller.write('\r');
  controller.write('\n');
}

void WiFi_setup() {
  //Using AP + STA mode, to allow automatic connection to the router while at home and dedicated network while on the field
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(wifi_ap_ip, wifi_ap_ip, wifi_ap_netmask);

  //construct the AP SSID based on the MAC address
  String ChipID = WiFi.macAddress();
  if (Use_AP_PASS) WiFi.softAP(AP_SSID + ChipID.substring(12, 14) + ChipID.substring(15, 17), AP_PASS);
  else  WiFi.softAP(AP_SSID + ChipID.substring(12, 14) + ChipID.substring(15, 17));

  if (!Use_DHCP) WiFi.config(wifi_sta_ip, wifi_sta_gateway, wifi_sta_netmask);
  WiFi.begin(STA_SSID, STA_PASS);

  //Wait for WiFi_connect_timeout seconds before deciding that home network is not in range
  uint32_t WiFiConnectTimer = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (millis() - WiFiConnectTimer >= WiFi_connect_timeout * 1000) break;
  }
  controller.println("Kx");
  server.begin(); // start TCP server
}

//when switching adapter modes between GPS and WiFi the baud rate also needs to be changed
void SwitchControllerBaud(int newBaud) {
  controller.flush();
  controller.end();
  controller.begin(newBaud);
}

void WiFi_handle() {
  //wait for a client to connect
  if (!client.connected()) { // if client not connected
    client = server.available(); // wait for it to connect
    return;
  }

  //a client request has been received
  if (client.available()) {
    while (client.available()) {
      buf1[i1] = (uint8_t)client.read();
      if (i1 < bufferSize - 1) i1++;
    }

    //switch baud rate if using the wrong one
    if (!CurrentMode) {
      SwitchControllerBaud(controller_baud_WiFi_mode);
      CurrentMode = true;
    }

    //send client request to handcontroller
    controller.write(buf1, i1);
    i1 = 0;

    //disable the GPS mode and start a client timeout timer
    GPS_terminated = true;
    WiFi_client_timer = millis();
  }

  if (controller.available()) {
    while (1) {
      if (controller.available()) {
        buf2[i2] = (char)controller.read(); // read char from UART
        if (i2 < bufferSize - 1) i2++;
      } else {
        //delayMicroseconds(packTimeoutMicros);
        delay(packTimeout);
        if (!controller.available()) {
          break;
        }
      }
    }
    client.write((char*)buf2, i2);
    i2 = 0;
  }
}

void GPS_handle() {
  //switch baud rate if using the wrong one
  if (CurrentMode) {
    SwitchControllerBaud(controller_baud_GPS_mode);
    CurrentMode = false;
  }

  //at initialization, start a timer used to delay the activation of WiFi
  if (!GPS_initialized) {
    GPS_initialized = true;
    GPS_initial_timer = millis();
  }

  // Read command from SynScan
  while (controller.available())
  {
    synscanEncode(controller.read());
  }

  gps.read();

  if (gps.newNMEAreceived()) {
    if (gps.parse(gps.lastNMEA()))
    {
      BinaryMsg binMsg = {0};
      uint8_t tmpBytes[4];

      binMsg.weekNo = 0; // not used
      binMsg.timeOfWeek = 0; // not used

      tmpBytes[0] = uint2bcd(gps.day);
      tmpBytes[1] = uint2bcd(gps.month);
      tmpBytes[2] = uint2bcd(gps.year);
      tmpBytes[3] = 0;
      memcpy(&(binMsg.date), tmpBytes, 4);

      tmpBytes[0] = uint2bcd(gps.seconds);
      tmpBytes[1] = uint2bcd(gps.minute);
      tmpBytes[2] = uint2bcd(gps.hour);
      tmpBytes[3] = 0;
      memcpy(&(binMsg.time), tmpBytes, 4);

      binMsg.latitude = (int32_t) (gps.latitudeDegrees / 360.0 * 4294967296);
      binMsg.longitude = (int32_t) (gps.longitudeDegrees / 360.0 * 4294967296);
      binMsg.altitude = (int16_t) gps.altitude;
      binMsg.heading = (uint16_t) gps.magvariation;
      binMsg.speed = (uint16_t) gps.speed;
      switch (gps.fixquality)
      {
        case 1 : binMsg.fixIndicator = 0; // GPS
        case 2 : binMsg.fixIndicator = 1; // DGPS
        default : binMsg.fixIndicator = 5; // Invalid
      }
      if (gps.fix)
      {
        binMsg.qualityOfFix = gps.fixquality_3d - 1; // 2D fix or 3D fix
      }
      else
      {
        binMsg.qualityOfFix = 0; // no fix
      }
      binMsg.numberOfSv = gps.satellites;
      binMsg.numberOfSvInFix = gps.satellites;
      binMsg.gdop = 1;
      binMsg.pdop = (uint8_t) gps.PDOP;
      binMsg.hdop = (uint8_t) gps.HDOP;
      binMsg.vdop = (uint8_t) gps.VDOP;
      binMsg.tdop = 1;

      if (sendBinaryMsg)
      {
        synscanSendBinMsg(&binMsg);
      }
    }
  }

  //if the GPS mode was active for GPS_initial_timer seconds, enable the WiFi mode
  if (millis() - GPS_initial_timer >= WiFi_init_delay * 1000) {
    WiFi_server_active = true;
  }
}
