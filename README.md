# Optoforce Driver
The current version of the library is built on the top of the official library of the manufacturer (see [here](http://optoforce.com/support/))

The access to the devices is done in 2 steps:
* all connected devices are first found.
* any connected devices an then be accessed to get the forces measured 

The class OptoforceApplication is proposed for applications looking for a basic recording of forces.

Examples of uses can be found in the directory [examples](examples).

Main developers so far: Asier Fernandez, Anthony Remazeilles


# Installation Instructions

## 1. Linux
- Add privileges for the user to "dialout" group
```bash
$ sudo usermod -a -G group_name user_name
```
- Change permission to usb port.
  * Create next file
```bash
$ sudo gedit /etc/udev/rules.d/72-OptoForce.rules
```
  * Copy his content
```bash
ATTR{idVendor}=="04d8", ATTR{idProduct}=="000a", MODE="0666", GROUP="dialout"
```


# To do
Most of the aspects to improve are defined in the code using the flag todo.
Othe relevant aspect to consider:
* Miguel started a driver in which a dirct access to the devices was done through boost (ie without the optofrce driver)
* An optoforce library has been developed [there](https://github.com/ethz-asl/liboptoforce)
* Optoforce developed a ROS node that should be tested as well.