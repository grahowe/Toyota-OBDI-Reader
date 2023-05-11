This code property of talofer99. THIS IS NOT MY CODE!!! I have reposted this with modifications that I made. This is now available with Imperial units (degrees F for coolant temp, psi for MAP pressure, MPH for car speed) but is STILL A W.I.P!!!!

Description:
This code was made for OBD1-compliant Toyotas and Lexuses with the VF1/ENG, TE2, and E1 pins found in the Diagnostic Link Connector. Logic output voltage is never above 5VDC, which is plenty for an Arduino board to handle if operated via USB or +5V on the VIN pin. My circuit, in contrast with talofer99's, consists of only two components - an Arduino board of your choice and a 4-pin OLED display. The display is connected via pins A4 (SDA) and A5 (SCL). This code features 3 displays - RPM, Full serial information, and binary flags. 

How Toyota's OBD1 system works:
The Toyota Serial Data Stream is comprised of 31 bits of information: 16 serial identifier bits, 4 OBD1 ID bits, and 11-bit words. We are only focused on the last 11 bits since these contain information about the car's state via the ECU. The ECU puts out 12 words per data stream.

Here is the information contained in the 12 11-bit data sequence sections:
* 0x00: N/A
* 0x01: Injector pulse width (INJ)
* 0x02: Ignition Timing Angle (IGN)
* 0x03: Idle Air Control (IAC)
* 0x04: Engine Revolutions (RPM)
* 0x05: Manifold Absolute Pressure (MAP, ***Now in PSI***)
* 0x06: Engine Coolant Temp (ECT, ***Now in deg. F***)
* 0x07: Throttle Position Sensor (TPS)
* 0x08: Vehicle Speed (SPD) ***Now in MPH***
* 0x09 & 0x10: N/A
* 0x11 & 0x12: Additional Binary Flags (Found in menu 3)

Keep in mind that this is not as fast as the OBDII protocol but will still provide real-time data. The delay time is around 1.25 seconds per data frame.

Great information for your reading and further research pleasure:
* https://forum.arduino.cc/t/reading-obd-1-data-from-toyota-corrola-1992/231151/39
* http://toyota.kgbconsulting.ca/wiki/OBD-1_Serial_Interface
* http://www.autoshop101.com/forms/h47.pdf
* https://www.yotatech.com/forums/attachments/f116/100816d1418667254-successful-93-obd-reading-obd-i-protocol-description.pdf
* https://www.mr2oc.co.uk/forum/toyobd1-basic-data-logging-for-pre-obd2-toyota-s-168964.html
* http://jfbreton.blogspot.com/search/label/OBD1READ
* http://jfbreton.blogspot.com/search/label/TOYOBD1
* https://www.reddit.com/r/arduino/comments/a1uq30/93_toyota_corolla_obd_i_system_328p_based/
* https://github.com/Stanneh1/TOYOBD1
* https://github.com/hyperion11/toyota-obd-1

CHANGE LOG:
* 5-10-2023: Made initial commits. As of right now, this code does not work unless the car is in the ON position. It will not work if the car has been started and is running for some reason. If someone can troubleshoot this on an OBD1-compatible Toyota, please post it in the issues, or feel free to contact me!
