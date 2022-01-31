#define AP_SSID "SynScan_WiFi_"
#define AP_PASS "12345678"
#define Use_AP_PASS false					//enable/disable the use of password for AP mode
#define STA_SSID "Home network SSID"
#define STA_PASS  "Home network password"
#define Use_DHCP true						//false = use custom IP address for STA mode, true = use IP assigned by router

#define OTA_pin 5							//pin used to trigger OTA update mode

#define GPS_module_baud 38400				//baud rate for GPS module, needs to be set according to the module's specifications
#define controller_baud_GPS_mode 4800		//baud rate for GPS mode, 4800 is used by SynScan handcontrollers
#define controller_baud_WiFi_mode 9600		//baud rate for WiFi mode, 9600 is used by SynScan handcontrollers
#define tcp_port  4030
#define packTimeout 5 						// ms (if nothing more on UART, then send packet)
#define bufferSize 8192

#define WiFi_init_delay 30      			// s (the WiFi is not active in the beginning, to allow some GPS to be read by the controller)
#define WiFi_connect_timeout 10 			// s (if no connection is made in STA mode, it will remain only in AP mode)
#define WiFi_client_timeout 30  			// s (if no client request is made, the GPS mode will be reactivated)

WiFiServer server(tcp_port);
WiFiClient client;

IPAddress wifi_ap_ip(10, 0, 0, 1);			//do not change, it is the default IP searched by dedicated software, like Stellarium
IPAddress wifi_ap_netmask(255, 255, 255, 0);
IPAddress wifi_sta_ip(192, 168, 0, 2);		//relevant only if you don't have DHCP; if custom IP address needed, change Use_DHCP to false
IPAddress wifi_sta_gateway(192, 168, 0, 1);
IPAddress wifi_sta_netmask(255, 255, 255, 0);
