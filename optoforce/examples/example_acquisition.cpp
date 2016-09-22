/**
 * @file   example_aquisition.cpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2016 Tecnalia Research & Innovation.
 * Distributed under the GNU GPL v3. For full terms see https://www.gnu.org/licenses/gpl.txt
 *
 * @brief Sample of data acquisition. Numer of sensors provided at launch.
 *
 */

#include <optoforce/optoforce_acquisition.hpp>
#include <signal.h>

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
  std::cout << "bin_force_acq [num_device] [num_samples]" << std::endl;
  std::cout << "[num_devices] number of devices expected" << std::endl;
  std::cout << "[num_samples] nb of samples to record" << std::endl;
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
  int connectedDAQs = atoi(argv[1]);
  int num_samples = -1;
  if (argc == 3)
    num_samples = atoi(argv[2]);

  /****************************************************/
  std::cout << "Looking for " << connectedDAQs << " connected DAQ " << std::endl;
  force_acquisition = new OptoforceAcquisition();

  if (!force_acquisition->initDevices(connectedDAQs))
  {
    std::cerr << "Something went wrong during initialization. /n Bye" << std::endl;
    return -1;
  }

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
