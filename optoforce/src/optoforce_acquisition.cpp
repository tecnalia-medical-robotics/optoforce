/**
 * @file   optoforce_aquisition.cpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2015 Tecnalia Research and Innovation.
 *
 * @brief connect to optoforce and acquire data to be stroed in a file.
 *
 */

#include "optoforce/optoforce_acquisition.hpp"
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

OptoforceAcquisition::OptoforceAcquisition() : device_enumerator_(NULL),
                                               is_recording_(false),
                                               is_stop_request_(false),
                                               max_num_samples_(500)
{

}

OptoforceAcquisition::~OptoforceAcquisition()
{
  std::cout << " Forcing the application to quit" << std::endl;
  if (isRecording())
  {
    std::cout << "is recording... stopping it" << std::endl;
    stopRecording();
    std::cout << " acquisition stopped" << std::endl;
  }
  closeDevices();
  std::cout << "object getting destructed" << std::endl;
}

// todo handle the data desallocation on error
bool OptoforceAcquisition::initDevices(const int nb_devices)
{
  //check if some devices are already connected. If so, just close it
  //todo handle such capability
  device_enumerator_ = new OptoForceArrayDriver(nb_devices);

  // look for the expected devices.
  if (!device_enumerator_->WaitUntilPortsFound(500))
  {
    std::cerr << "\n Could not find the " << nb_devices << " expected DAQ in time" << std::endl;
    return false;
  }
  // the expected DAQ are found, we now create the devices to be used.
  // get the enumarated ports
  std::vector<OPort> ports = device_enumerator_->GetPorts();

  OptoForceDriver * new_device;
  for (std::vector<OPort>::const_iterator it = ports.begin();
       it != ports.end();
       ++it)
  {
    new_device = new OptoForceDriver();
    bool success = new_device->openDevice(*it);
    if (success)
    {
      // Printing information on the device
      std::cout << "Device Opened:" << std::endl <<
        "Com port: " << new_device->getComPortName() << std::endl <<
        "Device Name: " << new_device->getDeviceName() << std::endl <<
        "S/N: " << new_device->getSerialNumber() << std::endl;
      if (new_device->is3DSensor())
        std::cout << "3D sensor" << std::endl;
      else
        std::cout << "6D sensor" << std::endl;
      std::cout << "*******************************" << std::endl;

      devices_.push_back(new_device);
    }
    else
    {
      // The device could not be opened,
      // show informations about the error and free resources
      std::cerr << "Failed to open device: " << new_device->getComPortName() <<
        "*******************************" << std::endl << std::flush;
      delete new_device;
    }

    // per default, we record all data
    devices_recorded_ = devices_;

    if (devices_.size() == nb_devices)
    {
      return true;
    }
    // if we reach that point, we did not get as many devices as expected
    std::cerr << "Only " << devices_.size() << " instead of " <<  nb_devices << "could be connected" << std::endl;
    std::cerr << "Disconnected all" << std::endl;

    closeDevices();
    return false;
  }
}

bool OptoforceAcquisition::reorderDevices(const std::vector<std::string> &lserial_number)
{
  devices_recorded_.clear();

  bool is_ok = true;

  for (int i = 0 ; (is_ok) && (i < lserial_number.size()); ++i)
  {
    if (!isDeviceConnected(lserial_number[i]))
    {
      std::cerr << "Could not find device " << lserial_number[i] << std::endl;
      std::cerr << "stopping the reordering" << std::endl;
      is_ok = false;
    }
    else
    {
      bool found = false;
      int j = 0;
      while (!found)
      {
        found = !(lserial_number[i].compare(devices_[j]->getDeviceName()));
        if (!found)
          ++j;
      }
      // we assume here the device has been found
      devices_recorded_.push_back(devices_[j]);
    }
  }
  if (!is_ok)
    devices_recorded_ = devices_;
  return is_ok;
}

bool OptoforceAcquisition::setDeviceCalibration(const std::string & serial_number, const std::vector<float> & calib)
{
  // make sure the device is connected
  if (!isDeviceConnected(serial_number))
    return false;

  // device connected looking for its handler
  bool found = false;
  int j = 0;
  while (!found)
  {
    found = !(serial_number.compare(devices_[j]->getDeviceName()));
    if (!found)
      ++j;
  }
  return devices_[j]->setCalibration(calib);
}

void OptoforceAcquisition::closeDevices()
{
  for (std::vector<OptoForceDriver *>::iterator it = devices_.begin();
       it != devices_.end();
       ++it)
  {
    delete *it;
  }
  devices_.clear();
  devices_recorded_.clear();

  if (device_enumerator_ != NULL)
    delete device_enumerator_;
  device_enumerator_ = NULL;
}

// todo can we use the value of thread_acq_ to know whether it is active or not?
bool OptoforceAcquisition::startRecording(const int num_samples)
{
  bool is_recording;
  mutex_.lock();
  is_recording = is_recording_;
  mutex_.unlock();

  if (is_recording)
    return false;

  // not recording, launch it!
  mutex_.lock();
  is_recording_ = true;
  mutex_.unlock();

  boost::thread t(&OptoforceAcquisition::acquireThread, this, num_samples);

  return true;
}

void OptoforceAcquisition::acquireThread(const int desired_num_samples)
{
  // double check if the data_acquired variable is empty. If not force it
  if (!data_acquired_.empty())
  {
    std::cout << "Acquisition buffer not empty. Previous data are deleted " << std::endl;
    data_acquired_.clear();
  }

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    std::vector<std::vector<float> > val;
    data_acquired_.push_back(val);
  }
  // to store the values of a device
  std::vector<float> values;

  bool is_stop_request = false;

  int max_samples = max_num_samples_;
  if (desired_num_samples != -1)
    max_samples = desired_num_samples;

  for (num_samples_ = 0; (num_samples_ < max_samples) && !is_stop_request; ++num_samples_)
  {
    for (size_t i = 0; i < devices_recorded_.size(); ++i)
    {
      values.clear();

      if (devices_recorded_[i]->getData(values))
      {
        for (unsigned int j = 0; j < values.size(); ++j)
          std::cout << values[j] << " ";
      }
      else
      {
        std::cerr << "\n Prb while reading the sensor data " << i << std::endl;
      }
      data_acquired_[i].push_back(values);
    }
    std::cout << std::endl;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(20));
    //Sleep(20);
    mutex_.lock();
    is_stop_request = is_stop_request_;
    mutex_.unlock();
  }
  std::cout << "Recorded " << num_samples_ << " data" << std::endl;
  mutex_.lock();
  is_recording_ = false;
  mutex_.unlock();
  std::cout << "acquireThread end" << std::endl;
}


// todo can we use the value of thread_acq_ to know whether it is active or not?
bool OptoforceAcquisition::isRecording()
{
  bool is_recording;
  mutex_.lock();
  is_recording = is_recording_;
  mutex_.unlock();
  return is_recording;
}

bool OptoforceAcquisition::isDeviceConnected(const std::string serial_number)
{
  std::vector<OptoForceDriver *>::iterator it_device;

  bool is_device_found = false;

  for (it_device = devices_.begin();
       (!is_device_found) && (it_device != devices_.end());
       ++it_device)
  {
    is_device_found = !(serial_number.compare((*it_device)->getDeviceName()));
  }
  return is_device_found;
}

bool OptoforceAcquisition::stopRecording()
{
  mutex_.lock();
  is_stop_request_ = true;
  mutex_.unlock();

  // block until it stops recording
  while (isRecording())
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

  std::cout << "stopRecording end" << std::endl;
  return true;
}

// todo: implement other aquisition modes
// todo: how to handle missing datas?
void OptoforceAcquisition::acquireData()
{
  // double check if the data_acquired variable is empty. If not force it
  if (!data_acquired_.empty())
  {
    std::cout << "Acquisition buffer not empty. Previous data are deleted " << std::endl;
    data_acquired_.clear();
  }

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    std::vector<std::vector<float> > val;
    data_acquired_.push_back(val);
  }
  // to store the values of a device
  std::vector<float> values;

  for (unsigned int cpt = 0; cpt < 100; ++cpt)
  {
    for (size_t i = 0; i < devices_recorded_.size(); ++i)
    {
      values.clear();

      if (devices_recorded_[i]->getData(values))
      {
        for (unsigned int j = 0; j < values.size(); ++j)
          std::cout << values[j] << " ";
      }
      else
      {
        std::cerr << "\n Prb while reading the sensor data " << i << std::endl;
      }
      data_acquired_[i].push_back(values);
    }
    std::cout << std::endl;
    boost::this_thread::sleep_for(boost::chrono::milliseconds(20));
  }
  std::cout << "Recorded " << data_acquired_[0].size() << " data" << std::endl;
}

// warning may not be correctly working if acquisition asked while storing
// todo if no data recorded, get out directly
bool OptoforceAcquisition::storeData()
{

// make sure we are not recording
  bool is_recording;
  mutex_.lock();
  is_recording = is_recording_;
  mutex_.unlock();

  if (is_recording)
  {
    std::cerr << "Record is active" << std::endl;
    return false;
  }

  if (data_acquired_.empty() || data_acquired_[0].empty())
  {
    std::cerr << "No data to record" << std::endl;
    return false;
  }

  boost::posix_time::ptime posix_time =
    boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
      boost::posix_time::second_clock::local_time() );

  // We only take part of the posix time
  std::string name_file = boost::posix_time::to_iso_string(posix_time) + "_forces.csv";

  std::cout << "Storing filename: " << name_file << std::endl;
  std::cout << "Storing " << num_samples_ << " samples" << std::endl;

  std::ofstream file_handler;
  file_handler.open (name_file);
  // todo check if the file opening worked

  // prepare the csv format, accroding to the sensor connected
  std::stringstream oss;

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    if (devices_recorded_[i]->is3DSensor())
      oss << "f_x;f_y;f_z;";
    else
      oss << "f_x;f_y;f_z;t_x,t_y;t_z;";
  }
  std::cout << "csv header would be: " << oss.str() << std::endl;
  file_handler << oss.str() << std::endl;

  // Storing the data
  for (size_t s = 0; s < num_samples_; ++s)
  {
    oss.str("");
    for (size_t i = 0; i < devices_recorded_.size(); ++i)
    {
      if (data_acquired_[i][s].empty())
      {
        if (devices_recorded_[i]->is3DSensor())
          oss << ";;;";
        else
          oss << ";;;;;;";
      }
      else
      {
        for (size_t j = 0 ; j < data_acquired_[i][s].size(); ++j)
        {
          oss << data_acquired_[i][s][j] << ";";
        }
      }
    }
    oss << std::endl;
    file_handler << oss.str();
  }
  file_handler.close();
}

