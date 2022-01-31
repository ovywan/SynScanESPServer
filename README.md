# SynScan ESP8266 GPS + WiFi adapter


Wireless control SynScan compatible motorized mount, with GPS for automatic location and time update.

Based on the wonderful work of [Roman Hujer](https://github.com/romanhujer) and [tazounet](https://github.com/tazounet).

The main code used to treat the Wifi and GPS requests is taken with no significant changes from the following repositories: 
*  WiFi: [SynScanESPServer](https://github.com/romanhujer/SynScanESPServer) by [Roman Hujer](https://github.com/romanhujer)
*  GPS: [SynScanGPS](https://github.com/tazounet/SynScanGPS) by [tazounet](https://github.com/tazounet)

## Required components:

*  ESP8266 module with TX, RX and 2 additional pins (I used ESP-02)
*  GPS module with NMEA output (I used Ublox NEO-7)

## How to connect to WiFi:

*  If using the adapter while at home, the WiFi STA mode is activated. If no connection to the router is made within a certain time, this mode is disabled until reboot and only the WiFi AP mode can be used
*  WiFi AP mode is always available, regardless of the availability of a home network:
<pre>
	SSID:  SynScan_WiFi_ABCD		//	ABCD = last bytes from the ESP MAC
	Pass:  12345678				//	optional
	IP:  10.0.0.1
	TcpPort: 4030
</pre>

## How to use:

When the SynScan handcontroller (HC) is powered up, the ESP adapter boots in GPS mode and responds to the GPS requests sent by the HC.

After a 30s delay, the WiFi connection is started. The ESP works in AP + STA mode, so it will create an access point to which you can connect anytime, but it will also try to connect to the defined home network. If the connection with the router is not possible within 10s, ESP will use only the AP mode.

With the WiFi enabled, the ESP waits for a client request (from the software / app of your choice). While no client requests have been received, the ESP works in GPS mode, providing the time and location information to the HC. A client request will disable the GPS mode and the ESP will only treat WiFi requests.

If no WiFi request is received for 30s, the ESP will re-enable the GPS mode, while still listening for new WiFi requests.

The timers mentioned are configurable in code (_Config.h_).

The project replaces 2 different accessories (each bought sepparately and also not cheap) with a simple circuit, made from only 2 modules that are very cheap, an ESP8266 and a GPS module with serial output. They can be mounted inside a dedicated, external enclosure, in which case you need to include the additional circuitry mentioned in [SynScanESPServer](https://github.com/romanhujer/SynScanESPServer). The alternative is to put the 2 modules inside the HC enclosure, thus requiring only some wires, kapton tape for isolation and some hot glue.

If opting for the second solution, you need to have GPS module that has a wired antenna that is small enough to fit inside the HC enclosure, prefferably pointing upwards.

In any case, you should take into account that the GPS module needs some time to fix the location, especially if near building or vegetation, so eventhough the HC recognizes the connection, you might still need to wait for the actual time and location to be available. Just use the GPS menu of the HC to update the information.
