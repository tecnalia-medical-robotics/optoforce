# Optoforce Driver Description

The current version of the library is built on the top of the official library of the manufacturer (see [here](http://optoforce.com/support/))

This version of the driver allows to manage more than one OptoForce device together.

The access to the devices is done in 2 steps:

* all connected devices are first found.
* any connected devices an then be accessed to get the forces measured 

The class OptoforceApplication is proposed for applications looking for a basic recording of forces.

Examples of uses can be found in the directory [examples](optoforce/examples).

Main developers so far: Asier Fernandez, Anthony Remazeilles


## Devices configuration under Linux

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

## Get and build the code

This package is a cmake project. Follow instrunctions bellow to build the project

```bash
# prepare workspace
cd to_you_workspace
mkdir build
mkdir src
cd src

# clone project
git clone https://github.com/tecnalia-medical-robotics/optoforce.git

# change directory to the build directory
cd ../build

# generate Makefiles
cmake ../src/optoforce

# build
make
```

## Basic usage

As an exmple, run the following example
```bash

# change directory to build/optoforce. Next command assumens current directory is build
cd optoforce

# launch example. Before launching, be sure only 1 optoforce device is connected
./bin_array_opto_force 1

```

## Gnuplot

If it is desired to visualize recorded data with gnuplot, follow instructions bellow:

```bash
# Set column separator
set datafile separator ";"

# Set ranges. set autoscale(default)
set xrange [1:10]
set yrange [1:100]

# Set labels
set title 'Measured Force'      # plot title
set xlabel 'Time'               # x-axis label
set ylabel 'Force'              # y-axis label

# Plot data. For example 3 Force axis
plot 'filename.csv' using 1:2 with lines title "Fx", 'filename.csv' using 1:3 with lines title "Fz", 'filename.csv' using 1:4 with lines title "Fz" 
```

## Related Projetcs

ETH Zurich ASL: [OptoForce driver](https://github.com/ethz-asl/liboptoforce)
