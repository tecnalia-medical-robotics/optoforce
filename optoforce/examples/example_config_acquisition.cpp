/**
 * @file   example_config_aquisition.cpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2016 Tecnalia Research & Innovation.
 * Distributed under the GNU GPL v3. For full terms see https://www.gnu.org/licenses/gpl.txt
 *
 * @brief Sample of data acquisition, using a configuration file.
 *
 */

#include <optoforce/optoforce_acquisition.hpp>
#include <signal.h>
#include <iostream>
#include "yaml-cpp/yaml.h"

OptoforceAcquisition * force_acquisition;

void my_handler(int s)
{
  std::cout << "Caught signal" << s << std::endl;

  if (force_acquisition != NULL)
  {
    delete force_acquisition;
    force_acquisition = NULL;
  }
}

void usage()
{
  std::cout << "bin_force_acq [config_file]" << std::endl;
  std::cout << "[config_file] yaml file containing configuration parameters" << std::endl;
}

int main(int argc, char* argv[])
{
  signal (SIGINT,my_handler);

  if (argc < 2)
  {
    std::cerr << "should provide at least the number of device to connect" << std::endl;
    usage();
    return -1;
}

  int num_samples = -1;
  std::vector<std::string> ldevice;
  std::vector<std::vector<float> > lcalib;

  /****************************************************/
  /***                  Config checking             ***/

  YAML::Node baseNode = YAML::LoadFile(argv[1]);
  if (baseNode.IsNull())
  {
    std::cerr<< "Not able to load the defined file: " << argv[1] << std::endl;
    return -1;
  } //File Not Found?
  std::cout<< "Loading configuration file: " << argv[1] << " done" <<std::endl;

  // num samples expected

  if (baseNode["num_samples"])
  {
    num_samples = baseNode["num_samples"].as<int>();
    std::cout <<"Willing to read " << num_samples << std::endl;
  }
  else
  {
    std::cout <<"num_samples undefined, left to default: " << num_samples << std::endl;
  }

  int connectedDAQs = baseNode["devices"].size();
  std::cout << "Expecting "<< connectedDAQs << " devices " << std::endl;

  bool is_config_ok = true;

  try
  {
    for (int i = 0; i < connectedDAQs; ++i)
    {
      std::string device_name = baseNode["devices"][i]["name"].as<std::string>();
      std::cout << "[" << i << "].name =" << device_name << std::endl;

      ldevice.push_back(device_name);

      std::vector<float> calib;
      std::cout << "[" << i << "].calibration = [";
      for (int j = 0; j <  baseNode["devices"][i]["calibration"].size(); ++j)
      {
        calib.push_back(baseNode["devices"][i]["calibration"][j].as<float>());
        std::cout << calib.back() << " ";
      }
      std::cout << "]" << std::endl;

      lcalib.push_back(calib);
    }
  }
  catch (...)
  {
    std::cerr << "Prb while loading configuration parameters" << std::endl;
    is_config_ok = false;
  }

  /****************************************************/
  /***                  Config checking end         ***/

  if (!is_config_ok)
  {
    std::cerr << "Exiting the program" << std::endl;
    exit(1);
  }

  /****************************************************/
  std::cout << "Looking for " << connectedDAQs << " connected DAQ " << std::endl;
  force_acquisition = new OptoforceAcquisition();

  if (!force_acquisition->initDevices(connectedDAQs))
  {
    std::cerr << "Something went wrong during initialization. /n Bye" << std::endl;
    return -1;
  }

  for (int i = 0; i < connectedDAQs; ++i)
  {
    if (!force_acquisition->isDeviceConnected(ldevice[i]))
    {
      std::cerr << "Could not find device " << ldevice[i] << std::endl;
      std::cerr << "Quitting the application." << std::endl;
      delete force_acquisition;
      exit(1);
    }
  }

  // reordering the devices according to the config order.

  force_acquisition->startRecording(num_samples);

  while ((force_acquisition != NULL) && force_acquisition->isRecording())
  {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
  }
  force_acquisition->storeData();

  std::cout << "main call delete"<<std::endl;
  delete force_acquisition;
  std::cout << "end main"<<std::endl;
}
