#include <iostream>

#include "optoforce/optoforce_array_driver.hpp"

#ifdef _WINDOWS
#include <windows.h>
//#include <mmsystem.h>
//#include "stdafx.h"
#else
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif // WINDOWS


OptoForceArrayDriver::OptoForceArrayDriver()
  : m_PortEnumerator(new OptoPorts()), m_PortCount(0)
{
}

/*
 * The p_portCount parameter tells the enumerator how many DAQs you
 * have connected to the computer.
 */
OptoForceArrayDriver::OptoForceArrayDriver(int p_portCount)
  : m_PortEnumerator(new OptoPorts()), m_PortCount(p_portCount)
{
}

OptoForceArrayDriver::~OptoForceArrayDriver()
{
  std::cout << "closing the optoports" << std::endl;
  delete m_PortEnumerator;
}

std::vector<OPort> OptoForceArrayDriver::GetPorts()
{
  std::vector<OPort> result;
  int iSize = m_PortEnumerator->getSize(true);
  if (iSize > 0) {
    OPort * portList = m_PortEnumerator->listPorts(true);
    iSize = m_PortEnumerator->getLastSize(); // this is the exact number of ports in portList after calling listPorts
    for (int i = 0; i < iSize; ++i) {
      result.push_back(portList[i]);
    }
  }
  return result;
}

int OptoForceArrayDriver::GetFoundPortsCount()
{
  return m_PortEnumerator->getSize(true);
}

/*
 * This function returns true if Enumerator has found all the ports you need,
 * or return false if timeout occured.
 * p_timeOut is the timeout in milliseconds
 */
bool OptoForceArrayDriver::WaitUntilPortsFound(unsigned long p_timeOut)
{
  if (m_PortCount == 0)
  {
    return true;
  }

  int nb_port;
  for (unsigned long num_trial = 0; num_trial < p_timeOut; num_trial++)
  {
    nb_port = GetFoundPortsCount();
    if (nb_port == m_PortCount)
    {
      return true;
    }
    if (num_trial > p_timeOut)
      return false;
    std::cout << "\r[" << num_trial << "/" << p_timeOut<< "]" << "num port found: " << nb_port << std::flush;
    Sleep(10); // To  not use all CPU resources
  }
  return false;
}
