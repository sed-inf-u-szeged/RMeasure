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

#include "PicoScopeModel.h"

namespace repara {
namespace measurement {
namespace scope {

Channel::Channel(const std::string& name, const std::map<std::string, xmlrpc_c::value>& settingsMap)
{
    _name = name;
    std::map<std::string, xmlrpc_c::value>::const_iterator settingsIt;

    settingsIt = settingsMap.find("hppdl");
    _hppdl = (settingsIt != settingsMap.end()) ?  static_cast<std::string>(xmlrpc_c::value_string(settingsIt->second)) : "";

    settingsIt = settingsMap.find("coupling");
    _coupling = (settingsIt != settingsMap.end()) ? static_cast<int>(xmlrpc_c::value_int(settingsIt->second)) : 0;

    settingsIt = settingsMap.find("range");
    _range = (settingsIt != settingsMap.end()) ? static_cast<int>(xmlrpc_c::value_int(settingsIt->second)) : 0;

    settingsIt = settingsMap.find("isEnabled");
    _enabled = (settingsIt != settingsMap.end()) ? static_cast<bool>(xmlrpc_c::value_boolean(settingsIt->second)) : false;

    settingsIt = settingsMap.find("analogOffset");
    _enabled = (settingsIt != settingsMap.end()) ? static_cast<double>(xmlrpc_c::value_double(settingsIt->second)) : 0.0;

    settingsIt = settingsMap.find("resistance");
    _resistance = (settingsIt != settingsMap.end()) ? static_cast<double>(xmlrpc_c::value_double(settingsIt->second)) : 0.0;

    settingsIt = settingsMap.find("isParport");
    _enabled = (settingsIt != settingsMap.end()) ? static_cast<bool>(xmlrpc_c::value_boolean(settingsIt->second)) : false;
}

Channel::~Channel()
{
}

const std::string& Channel::hppdl() const
{
    return _hppdl;
}

const int& Channel::coupling() const
{
    return _coupling;
}

const int& Channel::range() const
{
    return _range;
}

const bool& Channel::isEnabled() const
{
    return _enabled;
}

const double& Channel::analogOffset() const
{
    return _analogOffset;
}

const double& Channel::resistance() const
{
    return _resistance;
}

const bool& Channel::isParport() const
{
    return _parport;
}

PicoScopeModel::PicoScopeModel() :
    _channels(),
    _driverVersion(),
    _usbVersion(),
    _hardwareVersion(),
    _variantNumber(),
    _batchAndSerial(),
    _calibrationDate(),
    _kernelVersion()
{
}

const ChannelVector& PicoScopeModel::channels() const
{
    return _channels;
}

const std::string& PicoScopeModel::driverVersion() const
{
    return _driverVersion;
}

const std::string& PicoScopeModel::usbVersion() const
{
    return _usbVersion;
}

const std::string& PicoScopeModel::hardwareVersion() const
{
    return _hardwareVersion;
}

const std::string& PicoScopeModel::variantNumber() const
{
    return _variantNumber;
}

const std::string& PicoScopeModel::batchAndSerialNumber() const
{
    return _batchAndSerial;
}

const std::string& PicoScopeModel::calibrationDate() const
{
    return _calibrationDate;
}

const std::string& PicoScopeModel::kernelVersion() const
{
    return _kernelVersion;
}

void PicoScopeModel::setVersions(const xmlrpc_c::value& versionInformation)
{
        const xmlrpc_c::value_struct structInfo = static_cast<xmlrpc_c::value_struct>(versionInformation);
        std::map<std::string, xmlrpc_c::value> versionMap(static_cast<std::map<std::string, xmlrpc_c::value> >(structInfo));
        _driverVersion = static_cast<std::string>(xmlrpc_c::value_string(versionMap["driverVersion"]));
        _usbVersion = static_cast<std::string>(xmlrpc_c::value_string(versionMap["usbVersion"]));
        _hardwareVersion = static_cast<std::string>(xmlrpc_c::value_string(versionMap["hardwareVersion"]));
        _variantNumber = static_cast<std::string>(xmlrpc_c::value_string(versionMap["variantNumber"]));
        _batchAndSerial = static_cast<std::string>(xmlrpc_c::value_string(versionMap["batchAndSerial"]));
        _calibrationDate = static_cast<std::string>(xmlrpc_c::value_string(versionMap["calibrationDate"]));
        _kernelVersion = static_cast<std::string>(xmlrpc_c::value_string(versionMap["kernelVersion"]));
}

void PicoScopeModel::fillChannels(const xmlrpc_c::value& channelsInformation)
{
        const xmlrpc_c::value_struct channelsInfo = static_cast<xmlrpc_c::value_struct>(channelsInformation);
        std::map<std::string, xmlrpc_c::value> channelsMap(static_cast<std::map<std::string, xmlrpc_c::value> >(channelsInfo));

        std::map<std::string, xmlrpc_c::value>::iterator channelsIt = channelsMap.begin();
        for (;channelsIt != channelsMap.end(); ++channelsIt) {
            const std::string channelName = static_cast<std::string>(xmlrpc_c::value_string(channelsIt->first));
            const xmlrpc_c::value_struct settingsInfo = static_cast<xmlrpc_c::value_struct>(channelsIt->second);
            std::map<std::string, xmlrpc_c::value> settingsMap(static_cast<std::map<std::string, xmlrpc_c::value> >(settingsInfo));
            Channel channel(channelName, settingsMap);
            _channels.push_back(channel);
        }
}

} // namespace repara::measurement::scope
} // namespace repara::measurement
} // namespace repara
