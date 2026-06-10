- Test power
- How many of these can an Arduino Nano power?
# Convert to DC motor
- Convertion
	- https://www.youtube.com/watch?v=rifzrq5KiEs
	- https://www.youtube.com/watch?v=JhHSXCLsN4k
	- Remove the connection between the drive shaft and the potmeter
	- Grind the physical limiter on the drive shaft
	- Replace the potmeter with a voltage divider of 2k2+2k2 or 5k+5k
		- Middle pin is the swiper
	- Trim the potmeter close to 90 degrees
		- myServo.writeMicroseconds(1500)
	- Find the actual stop position with a loop around 1500
		- 1500-300, 1500+300 in steps of 10 and check when it actually stops
		- Record this neutral position
	- There is a range around the neutral position where the speed can actually be controlled now
- Both speed and direction can be controlled
	- 
# MG996R
- 4.8V to 7.2V
- Draws upto 2A per motor peak
- Using 5xAA (only rechargeable) works without buck convertor
	- AI suggests to add 2 power diodes an capacitors, seems not necessary
- [ ] Can we control the speed of a MG996R via its signal?
1000 µs → full speed left
1400 µs → slow left
1500 µs → stop
1600 µs → slow right
2000 µs → full speed right
# 130DC motor
- Should use a P-channel mosfet to drive
