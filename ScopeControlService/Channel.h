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

#ifndef CHANNEL_H_INCLUDED
#define CHANNEL_H_INCLUDED

#include <ps4000aApi.h>
#include <string>
#include <utility>
#include <vector>

/**
 * Namespace for the PicoScope implementation
 */
namespace ps4000a {

/**
 *
 * A class what is responsible for the channel information.
 */
class Channel
{
    PS4000A_CHANNEL m_channelType; ///< type of the channel
    std::string m_hppdl; ///< HPP-DL component id (measured where)
    PS4000A_COUPLING m_coupling; ///< type specifies the coupling mode: DC or AC
    PS4000A_RANGE m_range; ///<  specifies the measuring range
    bool m_enabled; ///< specifies whether the channel is active
    double m_analogOffset; ///< specifies the voltage in volts, to be added to the input signal before it reaches the input amplifier and digitizer
    double m_resistance; ///< specifies the value of the measurement resistor (in ohm)
    double m_gain; ///< specifies the gain of the amplifier
    bool m_parport; ///< specifies whether the channel is measure a parallel port


    /*
     * Convert millivolt (integer type) to PS4000A_RANGE type
     */
    PS4000A_RANGE millivoltsToRange(const int& mVolts) const;

    /*
     * Convert PS4000A_RANGE type to millivolt (integer type)
     */
    int rangeToMilliVolts(const PS4000A_RANGE& range) const;

public:

    Channel(const int number,const std::string& hppdl, const int coupling,
        const int range, const bool enabled, const double analogOffset,
        const double resistance, const double gain, const bool parport);

    const PS4000A_CHANNEL& channelType() const;
    const std::string& hppdl() const;
    const PS4000A_COUPLING& coupling() const;
    const PS4000A_RANGE& range() const;
    const bool& isEnabled() const;
    const double& analogOffset() const;
    const double& resistance() const;
    const double& gain() const;
    const bool& isParport() const;
    int rangeInt() const;
    std::string channelTypeName() const;

};

/**
 * A vector for the channel settings.
 */
typedef std::vector<Channel> ChannelVector;
typedef std::pair <PS4000A_CHANNEL,int> ParPortInfo;

} // namespace ps4000a

#endif // CHANNELSETTINGS_H_INCLUDED
