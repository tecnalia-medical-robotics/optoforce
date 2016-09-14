## Optoforce Driver Description

The current version of the library is built on the top of the official library of the manufacturer (see [here](http://optoforce.com/support/))

This version of the driver allows to manage more than one OptoForce device together.

The access to the devices is done in 2 steps:

* all connected devices are first found.
* any connected devices an then be accessed to get the forces measured 

The class OptoforceApplication is proposed for applications looking for a basic recording of forces.

Examples of uses can be found in the directory [examples](examples).

Main developers so far: Asier Fernandez, Anthony Remazeilles


## Devices configuration in Linux

- Add privileges for the user to "dialout" group
```bash
$ sudo usermod -a -G dialout user_name
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

## Gnuplot

- Set column separator

    set datafile separator ";"

- Set ranges
    
    set autoscale        # let gnuplot determine ranges (default)

    set xrange [1:10]
    
    set yrange [1:100]

- Set labels

    set title 'Measured Force'      # plot title

    set xlabel 'Time'               # x-axis label

    set ylabel 'Force'              # y-axis label

- Plot data. Only 1 Force axis
    
    plot 'filename.csv' using 1:2 with lines

- Plot data. 3 Force axis

    plot 'filename.csv' using 1:2 with lines title "Fx", 'filename.csv' using 1:3 with lines title "Fz", 'filename.csv' using 1:4 with lines title "Fz" 

## Related Projetcs

OptoForce: [EtherDAQ ROS driver](https://github.com/OptoForce/etherdaq_ros)

Shadow Robot: [ROS-Serial driver](https://github.com/shadow-robot/optoforce/blob/indigo-devel/optoforce/src/optoforce/optoforce.py)

LARICS-Lab: [OMD based ROS driver](https://github.com/larics/optoforce-ros-pusblisher)

ETH Zurich ASL: [OptoForce driver](https://github.com/ethz-asl/liboptoforce)
