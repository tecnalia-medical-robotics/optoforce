// MoreAdvancedExample.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <vector>
#include <iostream>
//#include <windows.h>
#include "optoforce/optoforce_driver.hpp"
#include "optoforce/optoforce_array_driver.hpp"
#include "omd/optoports.h"
#ifdef _WINDOWS
#include <windows.h>
//#include <mmsystem.h>
//#include "stdafx.h"
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif // WINDOWS

// this is likely to be specific to linux
#include <signal.h>

std::vector<OptoForceDriver *> devices3D;
std::vector<OptoForceDriver *> devices6D;

OptoForceArrayDriver * deviceEnumerator = NULL;

void my_handler(int s){
  std::cout << "Caught signal" << s << std::endl;
  // Release all the resources
  for (std::vector<OptoForceDriver *>::iterator it = devices3D.begin(); it != devices3D.end(); ++it) {
    delete *it;
  }
  for (std::vector<OptoForceDriver *>::iterator it = devices6D.begin(); it != devices6D.end(); ++it) {
    delete *it;
  }
  devices3D.clear();
  devices6D.clear();

  delete deviceEnumerator;
  exit(1);
}


int main(int argc, char* argv[])
{
  signal (SIGINT,my_handler);

  if (argc != 2)
  {
    std::cerr << "should provide the number of device to connect" << std::endl;
    return -1;
  }

  /*This value should be changed to fulfill your needs */
  // This is the number of DAQs you want to use

  int connectedDAQs = atoi(argv[1]);
  /****************************************************/
  std::cout << "Looking for " << connectedDAQs << " connected DAQ " << std::endl;
  /*
   * The enumerator which can list all of connected DAQs
   * Please note that only one instance is allowed per process otherwise poor performance is guaranteed
   */
  deviceEnumerator = new OptoForceArrayDriver(connectedDAQs);

  /*
   * We store the devices in vectors
   */

  /*
   * Wait for the enumerator to enumerate all the devices you want to use (time is given in milliseconds)
   * (I've tested the example with 3 sensors - one 3D sensor and to 6D sensors - and 5000 ms was enough)
   */
  if (deviceEnumerator->WaitUntilPortsFound(500) == false) {
    std::cout << "Could not able to enumerate all connected DAQs in time" << std::endl << std::flush;
    return 0;
  }

  /*
   * Get the enumerated ports
   */
  std::vector<OPort> ports = deviceEnumerator->GetPorts();

  /*
   * Create devices
   */
  for (std::vector<OPort>::const_iterator it = ports.begin(); it != ports.end(); ++it) {
    OptoForceDriver * newDevice = new OptoForceDriver();
    bool success = newDevice->openDevice(*it);
    if (success) {
      /*
        The device could be opened so let's show some information about it
      */
      std::cout << "Device Opened:" << std::endl <<
        "Com port: " << newDevice->getComPortName() << std::endl <<
        "Device Name: " << newDevice->getDeviceName() << std::endl <<
        "S/N: " << newDevice->getSerialNumber() << std::endl <<
        "*******************************" << std::endl << std::flush;
      if (newDevice->is3DSensor()) {
        /*
         * The device is a 3D sensor, store it in the appropriate vector
         */
        devices3D.push_back(newDevice);
      }
      else {
        /*
         * The device is a 6D sensor, store it in the appropriate vector
         */
        devices6D.push_back(newDevice);
      }
    }
    else {
      /*
       * The device could not be opened, show the information about the error and free resources
       */
      std::cout << "Failed to open device: " << newDevice->getComPortName() <<
        "*******************************" << std::endl << std::flush;
      delete newDevice;
    }
  }

  std::vector<float> values;
  std::vector< std::vector<float> > data;
  // Do some operations with the devices

  // 1.- Set Transmission Speed
  devices6D[0]->setFrequency(speed_1000hz);

  int packet_counter = 0;
  int it_counter = 0;
  int lost_counter = 0;

  double loop_start_time, loop_end_time, loop_current_time;
  timespec current_timespec;
  clock_gettime(CLOCK_REALTIME, &current_timespec);

  loop_start_time = (double)current_timespec.tv_sec+current_timespec.tv_nsec/1000000000.0;
  loop_current_time = loop_start_time;

  // Make an initial acquisition to delete buffer
  devices6D[0]->getData(data);

  float acq_time = 5;
  int sleep_time = 1000;
  std::cout << "Start Acquisition" << std::endl;
  //for (unsigned int i = 0; i < 10000; ++i)
  while ( (loop_current_time-loop_start_time) <= acq_time)
  {
    /*
    std::cout << "[ iteration "  << i <<  "]"<< std::endl;
    int device_id = 0;
    for (std::vector<OptoForceDriver *>::iterator it = devices3D.begin(); it != devices3D.end(); ++it) {
      OptoForceDriver * device = *it;

      if (device->getData(values, device_id))
      {
        for (unsigned int j = 0; j < values.size(); ++j)
          std::cout << values[j] << " ";
        //device_id ++;
      }
      else
      {
        std::cerr << "Prb while reading the sensor data" << std::endl;
      }
    }
    std::cout << std::endl;
    */
    for (std::vector<OptoForceDriver *>::iterator it = devices6D.begin(); it != devices6D.end(); ++it) {
      OptoForceDriver * device = *it;

      // Get last Data on the buffer
      /*
      device->getData(values);
      for (unsigned int j = 0; j < values.size(); ++j)
        std::cout << values[j] << " ";
      std::cout << std::endl;
      */

      if (!device->getData(data))
      {
        lost_counter = lost_counter +1;
      }
      else
      {
        std::cout << "size: " << data.size() << std::endl;
        //counter = counter + data.size();
        packet_counter = packet_counter + data.size();

        /*
        for (unsigned int i = 0; i < data.size(); i++)
        {
          std::cout << "counter_" << counter << ":\t";
          for (unsigned int j = 0; j < data[i].size(); j++)
          {
            std::cout << data[i][j] << " ";
          }
          std::cout << std::endl;

          packet_counter = packet_counter +1;
        }*/
      }
    }
    Sleep(sleep_time);
    // Wait some time for new data
    clock_gettime(CLOCK_REALTIME, &current_timespec);
    loop_current_time = (double)current_timespec.tv_sec+current_timespec.tv_nsec/1000000000.0;
    //std::cout << "loop time: " << loop_current_time -  loop_start_time<< std::endl;

    it_counter = it_counter +1;
  }

  //make a las reading
  std::cout << "packet_counter before last: " << packet_counter << std::endl;
  devices6D[0]->getData(data);
  packet_counter = packet_counter + data.size();
  std::cout << "packet_counter after  last: " << packet_counter << std::endl;

  clock_gettime(CLOCK_REALTIME, &current_timespec);
  loop_end_time = (double)current_timespec.tv_sec+current_timespec.tv_nsec/1000000000.0;

  double loop_time = loop_end_time-loop_start_time;
  std::cout << "packet_counter:  " << packet_counter << std::endl;
  std::cout << "packet_expected: " << (loop_time*1000) << std::endl;
  std::cout << "it_counter: " << it_counter << std::endl;
  std::cout << "elapsed time: " << loop_time << std::endl;
  std::cout << "lost packets: " << lost_counter << std::endl;



  // Release all the resources
  for (std::vector<OptoForceDriver *>::iterator it = devices3D.begin(); it != devices3D.end(); ++it) {
    delete *it;
  }
  for (std::vector<OptoForceDriver *>::iterator it = devices6D.begin(); it != devices6D.end(); ++it) {
    delete *it;
  }
  devices3D.clear();
  devices6D.clear();

  delete deviceEnumerator;
  return 0;
}

