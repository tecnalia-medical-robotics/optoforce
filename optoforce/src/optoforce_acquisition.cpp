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

#include "optoforce/optoforce_acquisition.hpp"
#include <iostream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"

// the maximum number of samples is hardcoded to 5min supposing a 1Khz sensor acq
OptoforceAcquisition::OptoforceAcquisition() : device_enumerator_(NULL),
                                               is_recording_(false),
                                               is_reading_(false),
                                               is_stop_recording_request_(false),
                                               is_stop_reading_request_(false),
                                               is_start_recording_request_(false),
                                               auto_store_(true),
                                               max_num_samples_ (5 * 60 * 1000),
                                               acquisition_freq_(1000)
{
  filename_ = "";
  desired_num_samples_ = max_num_samples_;
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
      std::cerr << "Could not find device***" << lserial_number[i] << std::endl;
      std::cerr << "stopping the reordering" << std::endl;
      is_ok = false;
    }
    else
    {
      bool found = false;
      int j = 0;
      while (!found)
      {
        //found = !(lserial_number[i].compare(devices_[j]->getDeviceName()));
        found = !(lserial_number[i].compare(devices_[j]->getSerialNumber()));
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
    //found = !(serial_number.compare(devices_[j]->getDeviceName()));
    found = !(serial_number.compare(devices_[j]->getSerialNumber()));
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

bool OptoforceAcquisition::startRecording()
{
  mutex_.lock();
  is_start_recording_request_ = true;
  is_stop_recording_request_ = false;
  mutex_.unlock();

  std::cout << "[ OptoforceAcquisition::startRecording] is_start_recording_request: " << is_start_recording_request_ << std::endl;
  std::cout << "[ OptoforceAcquisition::startRecording] is_stop_recording_request: " << is_stop_recording_request_ << std::endl;

  mutex_.lock();
  is_recording_ = true;
  mutex_.unlock();

  if (!isReading())
    startReading();
}
// todo can we use the value of thread_acq_ to know whether it is active or not?
bool OptoforceAcquisition::startRecording(int num_samples)
{


}
// todo can we use the value of thread_acq_ to know whether it is active or not?
void OptoforceAcquisition::setAutoStore(bool auto_store)
{
  auto_store_ = auto_store;
}

// todo can we use the value of thread_acq_ to know whether it is active or not?
bool OptoforceAcquisition::startReading()
{
  std::cout << "startReading" << std::endl;
  std::cout << "is reading: " << is_reading_ << std::endl;
  bool is_reading;
  mutex_.lock();
  is_reading = is_reading_;
  mutex_.unlock();

  // If a recording is running, start new recording
  // stopRecording function will set the flag to break acquireThread while loop
  if (is_reading)
  {
    stopReading();
  }
    //return false;

  // not recording, launch it!
  mutex_.lock();
  is_reading_ = true;
  mutex_.unlock();

  std::cout << "launching thread" << std::endl;
  //boost::thread t(&OptoforceAcquisition::acquireThread, this, num_samples, false);
  //if (thread_acq_ != NULL)
    //thread_acq_.reset();


    thread_acq_ = boost::shared_ptr< boost::thread >(new boost::thread(boost::bind(&OptoforceAcquisition::acquireThread, this, false)));

  return true;
}

// return latest data
void OptoforceAcquisition::getData(std::vector< std::vector<float> > &latest_samples)
{
  mutex_sample_.lock();
  latest_samples = latest_samples_;
  mutex_sample_.unlock();
}

void OptoforceAcquisition::getSerialNumbers(std::vector<std::string> &serial_numbers)
{
  serial_numbers.clear();
  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    serial_numbers.push_back(devices_recorded_[i]->getSerialNumber());

  }
}

//todo store the gloabl start time, and then delay all data wrt to this time?
void OptoforceAcquisition::acquireThread(bool is_debug)
{
  std::cout << "acquireThread" << std::endl;

  // double check if the data_acquired variable is empty. If not force it
  if (!data_acquired_.empty())
  {
    std::cout << "Acquisition buffer not empty. Previous data are deleted " << std::endl;
    data_acquired_.clear();
  }

  // to locally store all values without time info
  std::vector< std::vector< std::vector<float> > > data_acq;
  // to store the last values provided by the device
  std::vector< std::vector<float> > buffered_values;

  latest_samples_.clear();
  std::vector<float> vec;
  vec.clear();
  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    data_acq.push_back(buffered_values);
    latest_samples_.push_back(vec);
  }

  bool is_stop_reading_request = false;

  num_samples_ = 0;

  // for the first one, we just read the last value
  // so that we flush the internal buffer.
  bool is_first = true;

  // record the start instants of each device
  std::vector< boost::chrono::high_resolution_clock::time_point> l_time_start;
  // record the last recoding of each devic
  std::vector< boost::chrono::high_resolution_clock::time_point> l_time_last;

  std::cout << "is_stop_request: " << is_stop_reading_request << std::endl;
  std::cout << "is_stop_recording_request_: " << is_stop_reading_request_ << std::endl;

  bool is_start_recording_request = false;
  bool is_stop_recording_request = false;

  while (!is_stop_reading_request)
  {
    l_time_last.clear();

    if (is_debug)
      std::cout << "[" << num_samples_ << "] " ;

    mutex_.lock();
    is_start_recording_request = is_start_recording_request_;
    is_stop_recording_request = is_stop_recording_request_;
    mutex_.unlock();

    for (size_t i = 0; i < devices_recorded_.size(); ++i)
    {
      //values.clear();
      buffered_values.clear();

      bool is_data_available = true;

      is_data_available  = devices_recorded_[i]->getData(buffered_values);
      //l_time_last.push_back(boost::chrono::high_resolution_clock::now());

      if (is_data_available)
      {
          int idx_last = buffered_values.size()- 1;

          // Fill latest data within this variable
          // This variable is used to return latest value within getData method
          mutex_sample_.lock();
          latest_samples_[i] = buffered_values[idx_last];
          mutex_sample_.unlock();

        if (is_start_recording_request)
        {
          l_time_last.push_back(boost::chrono::high_resolution_clock::now());

          // todo: make sure is_data_available is true, and some data is available
          if (is_first)
          {
            //todo make sure the lat value is indeed the good one to use.
            // read the last value of each device, so that we fluch the internal buffer

            int idx_last = buffered_values.size()- 1;
            data_acq[i].push_back(buffered_values[idx_last]);

            if (is_debug)
            {
              for (unsigned int k = 0; k < buffered_values[idx_last].size(); ++k)
                std::cout << buffered_values[idx_last][k] << " ";
              std::cout << " + ";
            }
          }

          else
          {
            for (unsigned int j = 0; j < buffered_values.size(); ++j)
            {
              data_acq[i].push_back(buffered_values[j]);

              if (is_debug)
              {
                // displaying the values read.
                for (unsigned int k = 0; k < buffered_values[j].size(); ++k)
                  std::cout << buffered_values[j][k] << " ";
                std::cout << " + ";
              }
            }
          }
          if (is_first)
            l_time_start = l_time_last;
          is_first = false;
        }
      }
      else
      {
        //std::cerr << "\n Prb while reading the sensor data " << i << std::endl;
      }
      if (is_debug)
        std::cout << " || ";

      num_samples_ = data_acq[i].size();
    }

    if (is_debug)
      std::cout << std::endl;

    //if (is_first)
    //  l_time_start = l_time_last;
    //is_first = false;

    if (is_stop_recording_request)
    {
      std::cout << "Recorded " << num_samples_ << " data" << std::endl;

      for (size_t i = 0; i < devices_recorded_.size(); ++i)
      {
        boost::chrono::nanoseconds acq_all_sample_ns = l_time_last[i] - l_time_start[i];
        boost::chrono::nanoseconds acq_one_sample_ns = boost::chrono::nanoseconds(acq_all_sample_ns.count() / (data_acq[i].size() -1));

        std::cout << "Device["<< i << "]-->"<< data_acq[i].size() << " samples" << std::endl;
        std::cout << " Acquisition time : " << acq_all_sample_ns.count() * 1e-6 << std::endl;
        std::cout << " num data acquired: "  << std::endl;
        std::cout << " time per acq: " << acq_one_sample_ns.count() * 1e-6 << std::endl;

        std::vector<SampleStamped> l_data_stamped;
        SampleStamped data_stamped;
        for (unsigned int j = 0; j < data_acq[i].size(); ++j)
        {
          data_stamped.sample = data_acq[i][j];
          data_stamped.acq_time = l_time_start[i] + boost::chrono::nanoseconds(j * acq_one_sample_ns.count() );

          l_data_stamped.push_back(data_stamped);
        }

        data_acquired_.push_back(l_data_stamped);
        //std::cout << "Storing " <<  data_acquired_[i].size() << " samples" << std::endl;

      }

      mutex_.lock();
      is_recording_ = false;
      mutex_.unlock();

      storeData();

      for (size_t i = 0; i < devices_recorded_.size(); ++i)
      {
        data_acq[i].clear();
        data_acquired_.clear();
      }
      // clear variables
      //latest_samples_.clear();
      //vec.clear();
      //data_acq.clear();
      mutex_.lock();
      is_stop_recording_request_ = false;
      mutex_.unlock();

      is_first = true;
    }

    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000/acquisition_freq_));
  }

  std::cout << "[acquireThread] end" << std::endl;

}


bool OptoforceAcquisition::isRecording()
{
  bool is_recording;
  mutex_.lock();
  is_recording = is_recording_;
  mutex_.unlock();
  return is_recording;
}

bool OptoforceAcquisition::isReading()
{
  bool is_reading;
  mutex_.lock();
  is_reading = is_reading_;
  mutex_.unlock();
  return is_reading;
}

bool OptoforceAcquisition::isDeviceConnected(const std::string serial_number)
{
  std::vector<OptoForceDriver *>::iterator it_device;

  bool is_device_found = false;

  for (it_device = devices_.begin();
       (!is_device_found) && (it_device != devices_.end());
       ++it_device)
  {
    //is_device_found = !(serial_number.compare((*it_device)->getDeviceName()));
      is_device_found = !(serial_number.compare((*it_device)->getSerialNumber()));
  }
  return is_device_found;
}

bool OptoforceAcquisition::stopReading()
{
  std::cout << "stopReading start" << std::endl;

  mutex_.lock();
  is_stop_reading_request_ = true;
  mutex_.unlock();

  std::cout << "[ OptoforceAcquisition::startRecording] is_stop_reading_request_: " << is_stop_reading_request_ << std::endl;

  // block until it stops reading
  while (isReading())
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

  std::cout << "stopReading end" << std::endl;
  return true;
}

bool OptoforceAcquisition::stopRecording()
{
  std::cout << "[OptoforceAcquisition::stopRecording] in" << std::endl;

  mutex_.lock();
  is_stop_recording_request_ = true;
  is_start_recording_request_ = false;
  mutex_.unlock();

  std::cout << "[ OptoforceAcquisition::stopRecording] is_start_recording_request: " << is_start_recording_request_ << std::endl;
  std::cout << "[ OptoforceAcquisition::stopRecording] is_stop_recording_request: " << is_stop_recording_request_ << std::endl;

  // block until it stops reading
  while (isRecording())
    boost::this_thread::sleep_for(boost::chrono::milliseconds(10));

  std::cout << "[OptoforceAcquisition::stopRecording] out" << std::endl;
  return true;
}

void OptoforceAcquisition::setFilename(std::string filename)
{
  filename_ = filename;
}


// warning may not be correctly working if acquisition asked while storing
// todo if no data recorded, get out directly
// todo agree on a precision for the data stored.
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
    if (data_acquired_.empty())
      std::cerr << "no data has been recorded at all-..." << std::endl;
    return false;
  }

  boost::posix_time::ptime posix_time =
    boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(
      boost::posix_time::second_clock::local_time() );

  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    // We only take part of the posix time
    //std::string name_file = boost::posix_time::to_iso_string(posix_time) + "_"
      std::string name_file = filename_ + "_"
                                + devices_recorded_[i]->getSerialNumber() + "_"
                                + boost::posix_time::to_iso_string(posix_time)
                                + "_forces.csv";

    std::cout << "Storing filename: " << name_file << std::endl;
    std::cout << "Storing " <<  data_acquired_[i].size() << " samples" << std::endl;

    std::ofstream file_handler;
    // todo check if the file opening worked
    file_handler.open (name_file);

    // prepare the csv format, according to the sensor connected
    std::stringstream oss;

    // '#' is for Gnuplot
    if (devices_recorded_[i]->is3DSensor())
      oss << "#t_ms;f_x;f_y;f_z;";
    else
      oss << "#t_ms;f_x;f_y;f_z;t_x;t_y;t_z;";

    file_handler << oss.str() << std::endl;

    // Storing the data
    // todo decide what to do on the record time...
    // we assume the first one is the first first one
    // an we take it as reference
    boost::chrono::high_resolution_clock::time_point time_zero;

    if (data_acquired_.empty() || data_acquired_[0].empty())
    {
      //nothing acqired? we take current time as reference. But something is wrong.
      time_zero = boost::chrono::high_resolution_clock::now();
    }
    else
      time_zero = data_acquired_[0][0].acq_time;

    for (size_t j = 0; j < data_acquired_[i].size(); ++j)
    {
      oss.str("");

      boost::chrono::nanoseconds rel_time_ms = data_acquired_[i][j].acq_time - time_zero;
      oss << rel_time_ms.count() * 1e-6 << ";";

      if (data_acquired_[i][j].sample.empty())
      {
        if (devices_recorded_[i]->is3DSensor())
          oss << ";;;";
        else
          oss << ";;;;;;";
      }
      else
      {
        for (size_t k = 0 ; k < data_acquired_[i][k].sample.size(); ++k)
        {
          oss << data_acquired_[i][j].sample[k] << ";";
        }
      }
      oss << std::endl;
      file_handler << oss.str();
    }
    file_handler.close();
  }

  std::cout << "[storeData] Clear Variables to free memory" << std::endl;

  //std::vector<std::vector<SampleStamped> >().swap(data_acquired_);
  //std::vector< std::vector<float> >().swap(latest_samples_);
  data_acquired_.shrink_to_fit();
  latest_samples_.shrink_to_fit();
  //data_acquired_.clear();
  //latest_samples_.clear();


  return true;
}
void OptoforceAcquisition::setDesiredNumberSamples(int desired_num_samples)
{
  std::cout << "***" << std::endl;
  desired_num_samples_ = desired_num_samples;
  std::cout << "***" << std::endl;
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
// TODO set filter frequency to all devices?
bool OptoforceAcquisition::setSensorFilter(int filter_freq)
{
  std::cout << "[OptoforceAcquisition::setSensorFilter] filter choosed: " << filter_freq << std::endl;

  unsigned int filter = 0;

  switch (filter_freq)
  {
    case 150:
      filter = filter_150hz;
      break;
    case 50:
      filter = filter_50hz;
      break;
    case 15:
      filter = filter_15hz;
      break;
    case 0:
      filter = no_filter;
      break;
    default:
      std::cout << "[OptoforceAcquisition::setSensorSpeed] Not valid filter " << filter << std::endl;
      return false;
  }
  bool state = true;
  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    state  = state & devices_recorded_[i]->setFiltering((sensor_filter)filter);
  }

  return state;
}

void OptoforceAcquisition::setAcquisitionFrequency(int freq)
{
  acquisition_freq_ = freq;
}

bool OptoforceAcquisition::setZero(int number)
{
  return false;
}

bool OptoforceAcquisition::setZeroAll()
{
  for (size_t i = 0; i < devices_recorded_.size(); ++i)
  {
    devices_recorded_[i]->setZeroAll();
  }
  return true;
}
