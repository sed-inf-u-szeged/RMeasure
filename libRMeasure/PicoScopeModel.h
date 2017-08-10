/*
Copyright (c) 2014-2017 University of Szeged

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PICOSCOPEMODEL_H_INCLUDED
#define PICOSCOPEMODEL_H_INCLUDED

#include <string>
#include <map>
#include <xmlrpc-c/client_simple.hpp>

namespace repara {
namespace measurement {
/**
 * Namespace for oscilloscope-based measurement method implementation(s).
 */
namespace scope {

/**
 *
 * A class that contains channel information.
 */
class Channel
{
    std::string _name; ///< name of the measured channel
    std::string _hppdl; ///< HPP-DL component id (measured where)
    int _coupling; ///< type specifies the coupling mode: DC or AC
    int _range; ///<  specifies the measuring range (in millivolts)
    bool _enabled; ///< specifies whether the channel is active
    double _analogOffset; ///< specifies the voltage in volts, to be added to the input signal before it reaches the input amplifier and digitizer
    double _resistance; ///< specifies the value of the measurement resistor (in ohm)
    bool _parport; ///< specifies whether the channel is measured a parallel port

public:
    Channel(const std::string& name, const std::map<std::string, xmlrpc_c::value>& settingsMap);
    ~Channel();

    /*
     * Ensure information about the name of the measured channel.
     */
    const std::string& name() const;

    /*
     * Ensure information about the HPP-DL component id (measured where).
     */
    const std::string& hppdl() const;

    /*
     * Ensure information about the coupling mode (DC or AC) value.
     */
    const int& coupling() const;

    /*
     * Ensure information about the measuring range (millivolt).
     */
    const int& range() const;

    /*
     * Ensure information about the value which specifies whether the channel is active.
     */
    const bool& isEnabled() const;

    /*
     * Ensure information about the voltage in volts, which is added to the input signal.
     */
    const double& analogOffset() const;

    /*
     * Ensure information about the value of the measurement resistor (in ohm).
     */
    const double& resistance() const;

    /*
     * Ensure information about whether the channel is measured a parallel port.
     */
    const bool& isParport() const;

};

/**
 * A mapping from Channel to channel settings.
 */
typedef std::vector<Channel> ChannelVector;

/**
 * Store information about the scope unit
 */
class PicoScopeModel
{
    ChannelVector _channels; ///< contains information about the scope channels characteristics
    std::string _driverVersion; ///< version number of PicoScope
    std::string _usbVersion; ///< type of USB connection to device
    std::string _hardwareVersion; ///< hardware version of device
    std::string _variantNumber; ///< variant number of device
    std::string _batchAndSerial; ///< batch and serial number of device
    std::string _calibrationDate; ///< calibration date of device
    std::string _kernelVersion; ///< version of kernel driver

public:
    PicoScopeModel();
    const ChannelVector& channels() const;
    const std::string& driverVersion() const;
    const std::string& usbVersion() const;
    const std::string& hardwareVersion() const;
    const std::string& variantNumber() const;
    const std::string& batchAndSerialNumber() const;
    const std::string& calibrationDate() const;
    const std::string& kernelVersion() const;

    void setVersions(const xmlrpc_c::value& versionInformation);
    void fillChannels(const xmlrpc_c::value& channelInformation);
}; // class PicoScopeModel

} // namespace repara::measurement::scope
} // namespace repara::measurement
} // namespace repara

#endif // PICOSCOPEMODEL_H_INCLUDED
