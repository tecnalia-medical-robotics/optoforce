/**
 * @file   optoforce_aquisition.hpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2016 Tecnalia Research & Innovation.
 * Distributed under the GNU GPL v3. For full terms see https://www.gnu.org/licenses/gpl.txt
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
    \brief Auto store data after acquisition
    \param
  */
  void setAutoStore(bool auto_store);

  /*!
    \brief launch the recording of data
    \param num_samples number of reading requested (-1 is unlimited)
  */
  bool startRecording(const int num_samples);
  /*!
    \brief launch the recording of data
    \param num_samples number of reading requested (-1 is unlimited)
  */
  bool startRecording();
  /*!
    \brief launch the reading of data
    \return true if the operation succeeded
  */
  bool startReading();
  /*!
    \brief acquisition thread
    \param desired_num_samples number of reading requested (-1 is unlimited)
    \param is_debug whether extra information is displayed during acquisition
   */
  void acquireThread(bool is_debug = false);

  void getData(std::vector< std::vector<float> > &latest_samples);

  //! Get Serial numbers of connected deviced
  void getSerialNumbers(std::vector<std::string> &serial_numbers);
  /*!
    \brief check whether a data storing is active
    \return true if being recording
   */
  bool isRecording();
  /*!
    \brief check whether a data reading is active
    \return true if being recording
   */
  bool isReading();
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
    \brief request the stop of the reading
    \return true if the operation succeeded, false otherwise
  */
  bool stopReading();
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
    \brief to set filter frequency of the optoforce device
    \param filter_freq filter frequency in Hz. Valid frequencies are: 0Hz, 15Hz, 50Hz, 150Hz
   */
  bool setSensorFilter(int filter_freq);


  /*!
    \brief to set Zero of the optoforce device
    \param
   */
  bool setZero(int number);

  /*!
    \brief to set Zero of All optoforce devices
    \param
   */
  bool setZeroAll();

  /*!
    \brief Set Acquisition Frequency
    \param  freq acquisition frequency in Hz.
            This frequency determines how often we get a new data. Independently from Sensor Transmission Rate
   */
  void setAcquisitionFrequency(int freq);

  /*!
    \brief Set Desired Number Samples
  */
  void setDesiredNumberSamples(int desired_num_samples);

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
  //! whether or not is being reading data
  bool is_reading_;
  //! whether or not a recording stop is requested
  bool is_stop_recording_request_;
  //! whether or not a reading stop is requested
  bool is_stop_reading_request_;
  //! whether or not a recording start is requested
  bool is_start_recording_request_;
  //! to access to critical data shared in multi-threads
  boost::mutex mutex_;

  //! to access to critical data shared in multi-threads
  boost::mutex mutex_sample_;

  //! acquisition thread;
  //boost::thread* thread_acq_;
  boost::shared_ptr<boost::thread> thread_acq_;

  //! desired number of samples to be read from sensor
  int desired_num_samples_;
  //! current sample number
  size_t num_samples_;
  //! maximum number of samples
  const int max_num_samples_;
  //! acquisition frequency
  int acquisition_freq_;
  //! filename of the stored data
  std::string filename_;
  //! Flag to indicate auto store data in a file after theacquisition finishes
  bool auto_store_;

  //! last sensor data
  std::vector< std::vector<float> > latest_samples_;

};

#endif // OPTOFORCE_ACQUISITION_HPP
