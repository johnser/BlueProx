# BlueProx
BlueProx is a proximity Bluetooth car key that locks and unlocks your car based on the signal strength of your smartphone's Bluetooth signal. It implements the Eddystone UID beacon, sips power and is easily adaptable to many different kinds of cars. 

## Purpose
The purpose of this project is to finally obtain my goal of only carrying my phone in my pocket. Originally inspired by using mobile payments to replace my wallet, the only remaining pocket hitchhiker was my car key. This fixes that. 


## Design Requirements

In order for this project to be successful, the following requirements were set:
 1. BlueProx will be the only piece of hardware, which can be easily removed and installed into a car.
	> This means that the phone will only be able to communicate with its built in radio, no hardware will be designed to be carried along side it. Software applications on the phone are acceptable.
	
	>This also means that the BlueProx cannot be permanently installed to a car's anti theft hardware (or in general) as that would make it difficult to remove/install. Furthermore, power needs to provided at all times - even when the car is off - such that BlueProx will unlock the car at all times.
	
 2. The power consumption of BlueProx must be low enough to never drain an average car battery within a month
	>I never go more than a month without driving, so this is to prevent the car from dying after returning from a vacation. It should be unplugged since the hardware needs to be easily removable but humans forget sometimes
	
	> Note that unless you have a deep cycle battery in your car, the manufactures of lead acid batteries recommend that you never go below 50% depth of discharge. For my group size 35 battery, that is no less than [12.20V](https://www.energymatters.com.au/components/battery-voltage-discharge/). 
	>Assuming a fully charged battery, BlueProx cannot consume more than ***250 Wh*** in a month (assuming that the battery has [44 Ah](https://www.batteryequivalents.com/group-35-batteries-dimensions-features-and-recommendations.html) at 12V)
 3. The digital key must be able to be moved from phone to phone without reprogramming the BlueProx 
	>This means that BlueProx cannot use Bluetooth MAC IDs to unlock or lock the car. Even if an Android phone's Bluetooth MAC didn't change regularly ([it does, since Android 8.0)](https://source.android.com/devices/tech/connect/wifi-mac-randomization), I can't use a Bluetooth MAC address as it would change from phone to phone. Ideally, some other method should be used to identify the digital key.
 

# Hardware
## ~~ESP32~~
![ESP32](https://user-images.githubusercontent.com/17463970/80718810-77ada500-8aaf-11ea-98d7-eb4f79fe619e.png)

Originally, an ESP32 was chosen for this project. It was low cost, low power* and had nice additional features like a Wi-Fi radio, lots of sample code and a heavily advertised "ultra-low power mode". 

 - *The ESP32 has four modes: Active, Modem Sleep, Light and Deep Sleep. The issue is that the Bluetooth radio is disabled in all modes besides Active mode. In Active mode, the ESP32 consumes ~500mA, which is 2.5W. 

In a month, the ESP32 would consume 1825 Wh. Thats over x7 our design requirement of 250 Wh! Or to put it a different way: **a dead car battery in 8 days**! 

Due to the power consumption of 2.5W, the ESP32 is dead in the water, and a lower power Bluetooth radio must be sourced to move forward. 
>Note: This is a real shame, because the ESP32 consumes only ~1mA in Light sleep. Too bad all the radios are unusable in this state.


## nRF52832 (RedBear Nano V2)
![RedBear Nano V2](https://user-images.githubusercontent.com/17463970/80718862-87c58480-8aaf-11ea-9eed-1038721b737f.png)

After discovering that the ESP32 couldn't be run in a low power state for this project, I wanted something without any extra features - the bare minimum - even just Bluetooth if possible. Lucky for me, the   [RedBear Nano V2](https://github.com/redbear/nRF5x/tree/master/nRF52832) was the perfect replacement. It is basically a Bluetooth radio with a low power ARM processor (clocked at 16 MHz) and was designed to consume a little power as possible. Another upside is that is is almost a third of the size of the ESP32, which will be invaluable when attempting to integrate this into a small case.

>Note: The Redbear Nano V2 is now discontinued, and it does not seem like Particle (which has bought out RedBear) will make a replacement at this time. I procured mine for about $20 each
## ODB-II Case
![Cheap OBDII Case](https://user-images.githubusercontent.com/17463970/80718884-9318b000-8aaf-11ea-964a-68a365e1a5d5.png)

In order obtain power at all times, 12V is provided from the OBD-II plug. Since ODB-II is a requirement for all cars from 1996 onward to test for environmental compliance, this could be implemented on any car made since then. 
>Note: 12V cigarette lighter outlets (AKA the automobile auxiliary power outlets) are not a good solution here. They are spring loaded, so they can get get disconnected with time or vibration, and most turn off after the car sits for a few minutes or so. 

The case pictured above was used for BlueProx and is easily available on eBay and Amazon.  I procured mine for around $4 each.
>
## 12V to 5V DC Converter
![FINE DC to DC Converter](https://user-images.githubusercontent.com/17463970/80718921-9f9d0880-8aaf-11ea-874c-2a16936cc301.png)

Technically the RedBear Nano V2 can accept up to a 12V input. However, the 12V from a car's electrical system can go up to 14V when the alternator is running, contains lots of electrical noise and could have large voltage transients on start up. It would be much smarter to have something that was designed to handle a car's 12V system. Since there are lots of options, I went to find the most efficient one. Luckily for me, I found [Arik Yavilevich's blog](https://blog.yavilevich.com/2017/03/efficient-dc-12v-to-5v-conversion-for-low-power-electronics-evaluation-of-six-modules/). 

> Arik's testing found that the "Fine" volatge regulator to be the most
> efficient at low current (87% efficient at 25mA!). This is a great
> choice as the RedBear Nano V2 consumes around 6mA at 5V.

The voltage regulator pictured above was used for BlueProx and is easily available on eBay and Amazon. I procured mine for about $1 each

## OEM Car Key
![OEM Car Key](https://user-images.githubusercontent.com/17463970/80718975-af1c5180-8aaf-11ea-943a-3157499f8bdb.png)

The first design requirement means that BlueProx cannot modify the car's security system. 
In order to interface with the anti-theft system, BlueProx will only communicate with the key fob and will emulate a button press to lock and unlock the car. 

> See the schematic section below for a wiring diagram. Note that the car key is unique to each car but the wiring diagram should be the same for any car with remote control but not keyless entry. 

Luckily for us, the RedBear Nano V2 has a 3V output, so we can power the remote without the battery. This also disables the key if the Nano V2 is not powered.

These clones of these remotes are easily available on eBay and Amazon at a very low cost. I had a spare, but one could be procured for around $20 each.

## Miscellaneous Parts

 - In order to program the RedBear Nano V2, a [Particle Debugger](https://store.particle.io/products/particle-debugger) will need to be purchased.  
- Three NPN transistors and resistors will also need to purchased. 
	> I picked the 2N2222 and 100kΩ resistors, but you should calculate your base resistance values based on full saturation of the transistor, collector current and the transistor gain.

# Schematic

![Schematic](https://user-images.githubusercontent.com/17463970/80719053-c3604e80-8aaf-11ea-9e9f-e5c690ba42ac.png)

Ignore the pin numbers on Nano V2 and OEM Key in the schematic above. All pins are labeled except for ground connections.

# Code
The code for this project was written in C++ in the Ardunio IDE. 
The RedBear Nano V2 will constantly scan for an [Eddystone UID](https://github.com/google/eddystone/tree/master/eddystone-uid). You can modify the code by changing the following values to match your Eddystone UID:

	uint8_t KeyName[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t KeyInstance[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
	uint8_t KeyInstance2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 

I personally recommend using [Beacon Simulator on the Google Play Store](https://play.google.com/store/apps/details?id=net.alea.beaconsimulator&hl=en_US) to generate and broadcast your beacon. 
> Note that using the Eddystone beacon here complies with design requirement 3. As long as you remember the Eddystone UID, you can broadcast it on any Android phone. 
>
All other variables at the top of the code can be modified or tuned depending on your phone's transmitter power, local interference, timeout length, etc.   
The code will scan and skip any Bluetooth advertising data that is under 20 bytes (since the Eddystone UID must have atleast that many). 
>KeyInstance2 in the code will trigger an additional function (open the trunk in this case) but its optional to use.

An interesting note is that since every bluetooth beacon is compared to the `KeyName` and `KeyInstance`  variables, there is a noti


# Pictures

It's not pretty on the inside, but here is the completed project!

![enter image description here](https://user-images.githubusercontent.com/17463970/80719105-da06a580-8aaf-11ea-8a75-2f5d93e09181.png)

 - Always make things that you can take apart! The OEM key and the Nano V2 have connectors/headers to allow for disassembly and reprogramming.
 - The DC to DC converter has electrical tape on its bottom and is secured to the OBD-II connector with adhesive. Its not easy to remove, but since the other two components have connectors, it's no big deal. 
 - The 2N2222 transistors and 100 kΩ resistors are directly soldered on the OEM key. There was no other way to fit everything  inside the OBD-II housing otherwise.

![enter image description here](https://user-images.githubusercontent.com/17463970/80719135-e4c13a80-8aaf-11ea-9e7b-873ea2ab90f6.png)
> Note that this isn't the final wiring harness of BlueProx. D4 is not connected in these pictures. The resistors were also wrapped in electrical tape in the final iteration  
# Conclusion
The only design requirement not addressed is power consumption. So how much power does BlueProx consume?
The RedBear Nano V2 consumes around 6mA while scanning for bluetooth devices. Assuming that the DC to DC converter is 80% efficient at 6mA, we would be using around 6 Wh a month. That means we beat the design requirement by a multiple of ***41***! Not too shabby. 
> In theory, this means that it would take 3 years to drain a lead acid battery using BlueProx. Obviously other factors would prevent this from ever actually occurring. 

In general, BlueProx works amazingly well. You can tune it for your comfort level and make the antenna as sensitive as you would like. The longest range I was able to get was around 150 ft and the shortest was a few inches. 
I can now leave my keys in the center console of my vehicle, walk away, and have it lock. Since the horn honks when the vehicle locks, you can be sure that the vehicle is secured when a horn is heard while you walk away. When returning, the vehicle unlocks,  I pull my keys from the center console and drive away,

BlueProx could be improved in the future by improving it's security. We didn't talk much about security since its not great at the moment. Currently, if you knew that a car was using an Eddystone UID, you could scan, spoof and enjoy an open car. This could be remedied by using an Eddystone EID, however that requires synchronization from the beacon and BlueProx. This would also increase the power consumption of BlueProx and may require additional hardware. Due to the unlikelihood that someone would be attempting a spoof, I would categorize this as a minor risk and will address the security if the risk increases in the future. 

If you enjoyed my work, check out my [website](https://www.johnsser.com)! 


<!--stackedit_data:
eyJoaXN0b3J5IjpbODYyNzIyMjY4LC0yMzgxMjU1MTRdfQ==
-->