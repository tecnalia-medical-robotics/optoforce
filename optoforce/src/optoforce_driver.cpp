#include <iostream>
#include <string>
#include "optoforce/optoforce_driver.hpp"
#include "omd/optopackage.h"
#include "omd/optopackage6d.h"
#include <cmath>
#include <limits>

OptoForceDriver::OptoForceDriver()
{
  daq_ = new OptoDAQ();
}

OptoForceDriver::~OptoForceDriver()
{
  std::cout << "closing the devices" << std::endl;
  closeDevice();
  delete daq_;
}

bool OptoForceDriver::isOpen() const
{
  return daq_->isOpen();
}

// todo check the verisons with Asier. In his code this was different
bool OptoForceDriver::openDevice(const OPort & p_Port)
{
  if (isOpen())
  {
    // DAQ already opened, first, we need to close it
    closeDevice();
  }
  daq_->open(p_Port);

  if (isOpen())
  {
    port_ = p_Port;

    // we check the sensor type
    opto_version optoVersion = daq_->getVersion();
    // sensor type deduced from DAQ's version number
    is_3D_sensor_ =  (optoVersion != _95 && optoVersion != _64);

    if (is_3D_sensor_)
      factor_.resize(3);
    else
      factor_.resize(6);
    for (size_t i = 0; i < factor_.size(); ++i)
      factor_[i] = 1.0;

    return true;
  }
  // if we reach that line, the device is not properlly connected.
  return false;
}

void OptoForceDriver::closeDevice()
{
  if (isOpen()) 
  {
    daq_->close();
  }
}

/*
 * This function reads data from our DAQ. The p_iSensorIndex tells the API
 * which sensor data we want to read (e.g. if we have a 4 channel DAQ then
 * we can read the 3th sensor's data with p_iSensorIndex = 2).
 * if we have a 6D sensor the p_iSensorIndex is ignored.
 * val: returned vector
 *      returned vector is basically a list. A list of vectors with Force Data
 *      First vector in the list represents oldest data in time
 */
bool OptoForceDriver::getData(std::vector< std::vector<float> > & val, int p_iSensorIndex )
{
  // Here we can do anything with our Device (e.g. read data)
  if (!isOpen())
  {
    return false;
  }
  val.clear();

  // depending on the type, we may read either the first format or the second one
  OptoPackage package3d;
  OptoPackage6D *package6d = NULL;
  int iSize;

  if (is_3D_sensor_)
  {
    // We have a 3D sensor
    // The 3th parameter set to false clears the DAQ's internal buffer.
    iSize = daq_->read(package3d, p_iSensorIndex, false);
  }
  else
  {
    // We have a 6D sensor
    // Read last Data available on the Buffer
    iSize = daq_->readAll6D(package6d, false);
  }

  if (iSize < 0)
  {
    if (iSize == -1)
      std::cerr << "Buffer is full" << std::endl;
    else if (iSize == -2)
      std::cerr << "DAQ is Closed" << std::endl;

    // Something went wrong, please read the online documentation about error codes. (http://www.optoforce.com/software/API/apidoc/)
    return false;
  }

  if (iSize == 0)
  {
    //std::cout << "No new data could be read! (3D)" << std::endl << std::flush;
    return false;
  }

  if (iSize > 0)
  {
    for (int i = 0; i < iSize; i++)
    {
      std::vector<float> data;
      data.clear();

      if (is_3D_sensor_)
      {
        data.push_back(package3d.x * factor_[0]);
        data.push_back(package3d.y * factor_[1]);
        data.push_back(package3d.z * factor_[2]);
      }
      else
      {
        data.push_back(package6d[i].Fx * factor_[0]);
        data.push_back(package6d[i].Fy * factor_[1]);
        data.push_back(package6d[i].Fz * factor_[2]);
        data.push_back(package6d[i].Tx * factor_[3]);
        data.push_back(package6d[i].Ty * factor_[4]);
        data.push_back(package6d[i].Tz * factor_[5]);
      }
      val.push_back(data);
    }
  }
  if (package6d != NULL)
    delete[] package6d;
  return true;
}



/*
 * This function reads data from our DAQ. The p_iSensorIndex tells the API
 * which sensor data we want to read (e.g. if we have a 4 channel DAQ then
 * we can read the 3th sensor's data with p_iSensorIndex = 2).
 * if we have a 6D sensor the p_iSensorIndex is ignored.
 */
bool OptoForceDriver::getData(std::vector<float> & val, int p_iSensorIndex )
{
  // Here we can do anything with our Device (e.g. read data)
  if (!isOpen()) 
  {
    return false;
  }
  val.clear();

  // depending on the type, we may read either the first format or the second one
  OptoPackage package3d;
  OptoPackage6D package6d;
  int iSize;
  
  if (is_3D_sensor_) 
  {
    // We have a 3D sensor
    // The 3th parameter set to false clears the DAQ's internal buffer.
    iSize = daq_->read(package3d, p_iSensorIndex, false);
  }
  else
  {
    // We have a 6D sensor
    // Read last Data available on the Buffer
    iSize = daq_->read6D(package6d, false);
  }
  
  if (iSize < 0) 
  {
    // Something went wrong, please read the online documentation about error codes. (http://www.optoforce.com/software/API/apidoc/)
    return false;
  }
  
  if (iSize == 0) 
  {
    std::cout << "No new data could be read! (3D)" << std::endl << std::flush;
    return false;
  }

  // now we can assume the value is > 0
  if (is_3D_sensor_) 
  {
    val.push_back(package3d.x * factor_[0]);
    val.push_back(package3d.y * factor_[1]);
    val.push_back(package3d.z * factor_[2]);
  }
  else
  {
    val.push_back(package6d.Fx * factor_[0]);
    val.push_back(package6d.Fy * factor_[1]);
    val.push_back(package6d.Fz * factor_[2]);
    val.push_back(package6d.Tx * factor_[3]);
    val.push_back(package6d.Ty * factor_[4]);
    val.push_back(package6d.Tz * factor_[5]);
  }
  
  return true;
}

std::string OptoForceDriver::getSerialNumber() const
{
  return std::string(port_.serialNumber);
}

std::string OptoForceDriver::getComPortName() const
{
  return std::string(port_.name);
}

std::string OptoForceDriver::getDeviceName() const
{
  return std::string(port_.deviceName);
}


bool OptoForceDriver::setCalibration(const std::vector<float> & factor)
{
  if (!isOpen())
  {
    return false;
  }
  bool is_data_ok = (((factor.size() == 3) && is_3D_sensor_) ||
		    (factor.size() == 6) && !is_3D_sensor_);

  if (!is_data_ok)
    return false;

  // check if no value is 0
  for (size_t i = 0; (i < factor.size()) && is_data_ok; ++i)
  {
    is_data_ok = (std::fabs(factor[i]) > std::numeric_limits<float>::epsilon());
  }
  if (!is_data_ok)
    return false;
  
  for (size_t i = 0; i < factor.size(); ++i)
  {
    factor_[i] = 1.0 / factor[i];
    //std::cout << "factor: " << factor_[i] << std::endl;
  }
  return true;
}

bool OptoForceDriver::setFiltering(const sensor_filter filter)
{
  //std::cout << "[OptoForceDriver::setFiltering] filter: " << filter << std::endl;
  if (!isOpen())
  {
    return false;
  }

  // start by recovering the current state, 
  // so that we can change only the relevant parameter
  SensorConfig sensor_config = daq_->getConfig();

  if (sensor_config.filter != filter)
  {
    sensor_config.filter = filter;
    return daq_->sendConfig(sensor_config);
  }
  return true;
}

bool OptoForceDriver::setFrequency(const sensor_speed freq)
{
  if (!isOpen())
  {
    return false;
  }

  // start by recovering the current state, 
  // so that we can change only the relevant parameter
  SensorConfig sensor_config = daq_->getConfig();

  if (sensor_config.speed != freq)
  {
    sensor_config.speed = freq;
    return daq_->sendConfig(sensor_config);
  }
  return true;
}

bool OptoForceDriver::setZeroAll()
{
  if (!isOpen())
  {
    return false;
  }
  daq_->zeroAll();
  return true;
}
bool OptoForceDriver::setZero(int number)
{
  if (!isOpen())
  {
    return false;
  }
  return daq_->zero(number);

}
