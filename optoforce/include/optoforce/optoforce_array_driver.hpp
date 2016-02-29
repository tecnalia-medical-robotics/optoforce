/**
 * @file   optoforce_array_driver.hpp
 * @author Anthony Remazeilles <anthony.remazeilles@tecnalia.com>
 * @date   2016
 *
 * Copyright 2015 Tecnalia Research and Innovation.
 *
 * @brief Used for detecting the list of Optoforce devices connected.
 *
 */
// todo pragma makes sense under linux?
#pragma once
#include <vector>
#include "omd/optoports.h"
/*!
  \class OptoForceArrayDriver Enabling the listing of connected optoforce daqs
 */
class OptoForceArrayDriver
{
public:
  //! basic constructor
  OptoForceArrayDriver();
  /*!
    \brief constructor with a given number of expected daq
    \param p_portCount number of expected connected daq
    \warning the value given is only stored, not yet used
    \todo remove that input parameter
  */
  OptoForceArrayDriver(int p_portCount);
  //! basic destructor (todo: why virtual?)
  virtual ~OptoForceArrayDriver();
  //! get the list of connected daq
  std::vector<OPort> GetPorts();
  //! returns with the number of found ports
  int GetFoundPortsCount();
  /*!
    \brief wait to detect the expected number of daq
    \param p_timeOut number of attempts
    \return true if at least the numebr of expected devices has been found
    \todo change to get a timeout in ms
  */
  bool WaitUntilPortsFound(unsigned long p_timeOut);
protected:
  //! connected port enumarator (from Optforce API)
  OptoPorts * m_PortEnumerator;
  //! desired number of connected ports
  int m_PortCount;
};

