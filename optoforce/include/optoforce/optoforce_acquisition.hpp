/**
 * @file   optoforce_aquisition.hpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2015 Tecnalia Research and Innovation.
 *
 * @brief connect to optoforce and acquire data to be stroed in a file.
 *
 */
#ifndef OPTOFORCE_ACQUISITION_HPP
#define OPTOFORCE_ACQUISITION_HPP

#include "optoforce/optoforce_driver.hpp"
#include "optoforce/optoforce_array_driver.hpp"
#include <vector>

#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>

/*!
  \class Abstraction of resources needed for acquiring data with optoforce devices.
 */
class OptoforceAcquisition
{
public:
  //! basic structure used to stored the values provided by the devices
  struct SampleStamped
  {
    boost::chrono::high_resolution_clock::time_point acq_time;
    std::vector<float> sample;
  };

  //! basic constructor
  OptoforceAcquisition();
  //! basic destructor
  ~OptoforceAcquisition();
  /*!
    \brief perform the connection to the devices
    \param nb_device number of devices expected
    \return true if it suceeded
    \todo provide other means of initialization, such as specific serial number
   */
  bool initDevices(const int nb_devices);
  /*!
    \brief reorder the devices according to a name given
    \param lserial_number ordered name we whish to handle
    \return true if the operation succeeded
  */
  bool reorderDevices(const std::vector<std::string> &lserial_number);
 /*!
    \brief Properlly disconnect the system
  */
  void closeDevices();
  /*!
    \brief launch th recording of data
    \param num_samples number of reading requested (-1 is unlimited)
  */
  bool startRecording(const int num_samples);
  /*!
    \brief acquisition thread
    \param desired_num_samples number of reading requested (-1 is unlimited)
    \param is_debug whether extra information is displayed during acquisition
   */
  void acquireThread(const int desired_num_samples, bool is_debug = false);
  //! check whether a data acquisition is active
  bool isRecording();
  /*!
    \brief check if a given device is effectively seen as connected
    \param serial_number identificator of the device of interest
  */
  bool isDeviceConnected(const std::string serial_number);
  /*!
    \brief request the stop of the recording
    \return true if the operation succeeded, false otherwise
    \warning blocking until the acquisition is effectively blocked
  */
  bool stopRecording();
  /*!
    \brief store the data previously recorded
    \return true if the operation succeeded
   */
  bool storeData();

  /*!
    \brief set filename to store data
   */
  void setFilename(std::string filename);

  /*!
    \brief to set the calibration data of a device
    \param serial_number the device id
    \param calib the list of calibraiton values
   */
  bool setDeviceCalibration(const std::string & serial_number, const std::vector<float> & calib);

  /*!
    \brief to set acqusisition speed of the optoforce device
    \param freq acquisition frequency in Hz. Valid frequencies are: 33Hz, 100Hz, 333Hz, 1000Hz
   */
  bool setSensorSpeed(int freq);

  /*!
    \brief Set Acquisition Frequency
    \param  freq acquisition frequency in Hz.
            This frequency determines how often we get a new data. Independently from Sensor Transmission Rate
   */
  void setAcquisitionFrequency(int freq);

private:
  //! enumerator of available devices
  OptoForceArrayDriver * device_enumerator_;
  //! list of connected devices
  std::vector<OptoForceDriver *> devices_;
  //! list of devices recorded
  std::vector<OptoForceDriver *> devices_recorded_;
  //! list of values read per connected devices (couldn't we define a structure for the values rea, or reuse the initial one?)
  std::vector<std::vector<SampleStamped> > data_acquired_;
  //! whether or not is being recording data
  bool is_recording_;
  //! whether or not a recording stop is requested
  bool is_stop_request_;
  //! to access to critical data shared in multi-threads
  boost::mutex mutex_;
  //! acquisition thread;
  boost::thread* thread_acq_;
  //! current sample number
  size_t num_samples_;
  //! maximum number of samples
  const int max_num_samples_;
  //! acquisition frequency
  int acquisition_freq_;
  //! filename of the stored data
  std::string filename_;
};

#endif // OPTOFORCE_ACQUISITION_HPP
