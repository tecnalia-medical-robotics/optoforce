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
                                               max_num_samples_(500),
                                               acquisition_freq_(1000)
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
//todo avoid hardcoding the frequency in it.
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
      new_device->setFrequency(speed_1000hz);
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

  boost::thread t(&OptoforceAcquisition::acquireThread, this, num_samples, false);

  return true;
}

void OptoforceAcquisition::acquireThread(const int desired_num_samples, bool is_debug)
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
  std::vector< std::vector<float> > buffered_values;

  bool is_stop_request = false;

  int max_samples = max_num_samples_;
  if (desired_num_samples != -1)
    max_samples = desired_num_samples;

  std::cout << "samples to read: " << max_samples << std::endl;
  num_samples_ = 0;

  // for the first one, we just read the last value
  // so that we flush the internal buffer.
  bool is_first = true;


  // this is kept in case we want to store the data acquisition of each information (DATA_STRUCT)
  // std::vector< std::vector< std::pair<boost::chrono::high_resolution_clock::time_point , std::vector<std::vector<float> > > > > lvalues;
  // std::pair<boost::chrono::high_resolution_clock::time_point ,std::vector<std::vector<float> > > sensor_read;
  // for (size_t i = 0; i < devices_recorded_.size(); ++i)
  // {
  //  std::vector< std::pair<boost::chrono::high_resolution_clock::time_point ,std::vector<std::vector<float> > >> val;
  //  lvalues.push_back(val);
  // }

  // record the start instant
  boost::chrono::high_resolution_clock::time_point tstart = boost::chrono::high_resolution_clock::now();
  boost::chrono::high_resolution_clock::time_point tlast  = tstart;

  while ((num_samples_ < max_samples) && !is_stop_request)
  {
    if (is_debug)
      std::cout << "[" << num_samples_ << "] " ;
    for (size_t i = 0; i < devices_recorded_.size(); ++i)
    {
      //values.clear();
      buffered_values.clear();

      bool is_data_available = true;

      is_data_available  = devices_recorded_[i]->getData(buffered_values);
      tlast = boost::chrono::high_resolution_clock::now();

      if (is_data_available)
      {
        // todo: make sure is_data_available is true, and some data is available
        if (is_first)
        {
          num_samples_ = num_samples_ + 1;
          //todo make sure the lat value is indeed the good one to use.
          // read the last value of each device, so that we fluch the internal buffer

          int idx_last = buffered_values.size()- 1;
          data_acquired_[i].push_back(buffered_values[idx_last]);

          if (is_debug)
          {
            for (unsigned int k = 0; k < buffered_values[idx_last].size(); ++k)
              std::cout << buffered_values[idx_last][k] << " ";
            std::cout << " + ";
          }
        }
        else
        {
          // see (DATA_STRUCT) comment
          //sensor_read.first  = boost::chrono::high_resolution_clock::now();
          //sensor_read.second = buffered_values;
          //lvalues[i].push_back(sensor_read);

          for (unsigned int j = 0; j < buffered_values.size(); ++j)
          {
            data_acquired_[i].push_back(buffered_values[j]);

            if (is_debug)
            {
              // displaying the values read.
              for (unsigned int k = 0; k < buffered_values[j].size(); ++k)
                std::cout << buffered_values[j][k] << " ";
              std::cout << " + ";
            }
          }
        }
      }
      else
      {
        //std::cerr << "\n Prb while reading the sensor data " << i << std::endl;
      }
      if (is_debug)
        std::cout << " || ";

      num_samples_ = std::max(num_samples_,  data_acquired_[i].size());

    }

    if (is_debug)
      std::cout << std::endl;

    is_first = false;

    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000/acquisition_freq_));
    //boost::this_thread::sleep_for(boost::chrono::nanoseconds(500000000/acquisition_freq_));
    mutex_.lock();
    is_stop_request = is_stop_request_;
    mutex_.unlock();
  }
  std::cout << "Recorded " << num_samples_ << " data" << std::endl;
  mutex_.lock();
  is_recording_ = false;
  mutex_.unlock();
  std::cout << "acquireThread end" << std::endl;

  boost::chrono::nanoseconds acq_time_ns = tlast - tstart;

  std::cout <<" Acquisition time : " << acq_time_ns.count() * 1e-6 << std::endl;
  std::cout <<"num data acquired: "  << std::endl;

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    std ::cout << "Device["<< i << "]-->"<< data_acquired_[i].size() << std::endl;
  }

 // see (DATA_STRUCT) comment
  /* for (int i = 0 ; i < lvalues[i].size(); ++i)
  {
    std::cout << "device num " << i << std::endl;

    for (int j = 0; j < lvalues[i].size(); ++j)
    {
      boost::chrono::nanoseconds ns = lvalues[i][j].first - tstart;

      std::cout << "[" << ns.count() / 1000000.0 << "]";

      for (unsigned int k = 0; k < lvalues[i][j].second.size(); ++k)
          {
            for (unsigned int l = 0; l < lvalues[i][j].second[k].size(); ++l)
              std::cout << lvalues[i][j].second[k][l] << " ";
            std::cout << " + ";
          }
      std::cout << std::endl;
    }
  }
  */

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

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    // We only take part of the posix time
    std::string name_file = boost::posix_time::to_iso_string(posix_time) + "_"
      + devices_recorded_[i]->getSerialNumber() + "_forces.csv";

    std::cout << "Storing filename: " << name_file << std::endl;
    std::cout << "Storing " << num_samples_ << " samples" << std::endl;

    std::ofstream file_handler;
    // todo check if the file opening worked
    file_handler.open (name_file);

    // prepare the csv format, accroding to the sensor connected
    std::stringstream oss;

    if (devices_recorded_[i]->is3DSensor())
      oss << "f_x;f_y;f_z;";
    else
      oss << "f_x;f_y;f_z;t_x,t_y;t_z;";

    file_handler << oss.str() << std::endl;

    // Storing the data
    for (size_t j = 0; j < data_acquired_[i].size(); ++j)
    {
      oss.str("");
      if (data_acquired_[i][j].empty())
      {
        if (devices_recorded_[i]->is3DSensor())
          oss << ";;;";
        else
          oss << ";;;;;;";
      }
      else
      {
        for (size_t k = 0 ; k < data_acquired_[i][k].size(); ++k)
        {
          oss << data_acquired_[i][j][k] << ";";
        }
      }
      oss << std::endl;
      file_handler << oss.str();
    }
    file_handler.close();
  }
  return true;
}

// TODO set frequcny to all devices?
bool OptoforceAcquisition::setSensorSpeed(int freq)
{
  //std::cout << "[OptoforceAcquisition::setSensorSpeed] freq choosed: " << freq << std::endl;

  unsigned int frequency = 0;

  switch (freq)
  {
    case 1000:
      frequency = speed_1000hz;
      break;
    case 333:
      frequency = speed_333hz;
      break;
    case 100:
      frequency = speed_100hz;
      break;
    case 30:
      frequency = speed_30hz;
      break;
    default:
      std::cout << "[OptoforceAcquisition::setSensorSpeed] Not valid freq " << freq << std::endl;
      return false;
  }
  bool state = true;
  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    state  = state & devices_recorded_[i]->setFrequency((sensor_speed)frequency);
  }

  return state;
}

void OptoforceAcquisition::setAcquisitionFrequency(int freq)
{
  acquisition_freq_ = freq;
}
