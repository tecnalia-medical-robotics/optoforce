/**
 * @file   optoforce_driver.hpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2015 Tecnalia Research and Innovation.
 *
 * @brief Handler of a communicaiont stream with an Optoforce sensor.
 * Inspired from Optoforce example.
 *
 */

// todo check if pragma once makes sense under Linux
#pragma once
#include <string>
#include "omd/optodaq.h"
#include "omd/optoports.h"
#include <vector>

/*!
  \class handler of a connection with an Optoforce Device.
 */
class OptoForceDriver
{
public:
  //! basic constructor
  OptoForceDriver();
  //! basic destuctor (todo why virtual?)
  virtual ~OptoForceDriver();
  /*!
    \brief open the device according to the port provided
    \param p_Port data of the port connected to the daq 
  */
  bool openDevice(const OPort & p_Port);
  //! close the device
  void closeDevice();
  /*!
    \brief get the latest data acquired by the device
    \param val the read values
    \param p_iSensorIndex to select the 3d sensor when several are connected on a single daq
    \return true if some values could be read
   */
  bool getData(std::vector<float> & val, int p_iSensorIndex = 0);

  /*!
    \brief  get buffer of data. This buffer contains all data between two calls to this function
    \param  val return parameter as a vector. Read values
            Returned vector is basically a list. A list of vectors with Force Data
            First vector in the list represents oldest data in time
    \param  p_iSensorIndex to select the 3d sensor when several are connected on a single daq
    \return true if some values could be read
   */
  bool getData(std::vector< std::vector<float> > & val, int p_iSensorIndex = 0);

  //! whether or not a connecteion with a daq is active
  bool isOpen() const;
  /*!
    \brief check if the sensor is 3d or 6D.
    \return true if a 3D sensor
    \warning makes only sense if already connected
   */
  inline bool is3DSensor() const {return is_3D_sensor_;};
  /*!
    \brief check if the sensor is 3d or 6D.
    \return true if a 6D sensor
    \warning makes only sense if already connected
   */
  inline bool is6DSensor() const {return ! is_3D_sensor_;};;
  //! return the serial number of the connected daq
  std::string getSerialNumber() const;
  //! return the port used for the connection
  std::string getComPortName() const;
  //! return the device name (todo: what does it provide?)
  std::string getDeviceName() const;
  /*!
    \brief set the calibration parameters
    \param dividing factor for the 3/6 entries.
    \warning only possible once the device is connectd
    \return true if the operation succeeded.
    \todo check with Asier if such calibration is enough
   */
  bool setCalibration(const std::vector<float> & factor);
  /*!
    \brief configure the desired filtering
    \param filter_freq the desired frequency according to the authorized values (no_filter = 0, filter_150hz, filter_50hz, filter_15hz)
    \return true if the operation succeeded
    \warning nees to be connected
    \todo check the indications provided in http://www.optoforce.com/software/API/apidoc/class_opto_d_a_q.html#a7b1655abace006335ac5f205e99e4df4
    \todo check what is the default value
  */
  bool setFiltering(const sensor_filter filter_freq);
  /*!
    \brief configure the acquisition frequency
    \param freq the desired frequency according to the authorized values (speed_1000hz = 0, speed_333hz, speed_100hz, speed_30hz)
    \return true if the operation succeeded
    \warning nees to be connected
    \todo check the indications provided in http://www.optoforce.com/software/API/apidoc/class_opto_d_a_q.html#a7b1655abace006335ac5f205e99e4df4
    \todo check what is the default value
  */
  bool setFrequency(const sensor_speed freq);

protected:
  //! daq of the device considered.
  OptoDAQ * daq_;
  //! specification of the port related to that device
  OPort port_;
  //! specify if it is a 3d or 6d sensor
  bool is_3D_sensor_;
  //! calibration factor being used
  std::vector<float> factor_;
};

