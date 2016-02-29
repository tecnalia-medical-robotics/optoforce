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
