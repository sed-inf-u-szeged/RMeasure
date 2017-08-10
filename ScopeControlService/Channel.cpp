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

#include "Channel.h"

/**
 * Namespace for the PicoScope implementation
 */
namespace ps4000a {

PS4000A_RANGE Channel::millivoltsToRange(const int& mVolts) const
{
    switch (mVolts) {
        case 10:
            return PS4000A_10MV;
        case 20:
            return PS4000A_20MV;
        case 50:
            return PS4000A_50MV;
        case 100:
            return PS4000A_100MV;
        case 200:
            return PS4000A_200MV;
        case 500:
            return PS4000A_500MV;
        case 1000:
            return PS4000A_1V;
        case 2000:
            return PS4000A_2V;
        case 5000:
            return PS4000A_5V;
        case 10000:
            return PS4000A_10V;
        case 20000:
            return PS4000A_20V;
         case 50000:
            return PS4000A_50V;
         case 100000:
            return PS4000A_100V;
         case 200000:
            return PS4000A_200V;
        default:
            //std::cout << "invalid voltage range" << std::endl;
            return PS4000A_1V;
            break;
    }
}

int Channel::rangeToMilliVolts(const PS4000A_RANGE& range) const
{
    switch (range) {
        case PS4000A_10MV:
            return 10;
        case PS4000A_20MV:
            return 20;
        case PS4000A_50MV:
            return 50;
        case PS4000A_100MV:
            return 100;
        case PS4000A_200MV:
            return 200;
        case PS4000A_500MV:
            return 500;
        case PS4000A_1V:
            return 1000;
        case PS4000A_2V:
            return 2000;
        case PS4000A_5V:
            return 5000;
        case PS4000A_10V:
            return 10000;
        case PS4000A_20V:
            return 20000;
         case PS4000A_50V:
            return 50000;
         case PS4000A_100V:
            return 100000;
         case PS4000A_200V:
            return 200000;
        default:
            // std::cout << "invalid voltage range" << std::endl;
            return -1;
            break;
    }
}

Channel::Channel(const int number,const std::string& hppdl, const int coupling,
        const int range, const bool enabled, const double analogOffset,
        const double resistance, const double gain, const bool parport) :
    m_channelType((PS4000A_CHANNEL)number),
    m_hppdl(hppdl),
    m_coupling((PS4000A_COUPLING)coupling),
    m_range(millivoltsToRange(range)),
    m_enabled(enabled),
    m_analogOffset(analogOffset),
    m_resistance(resistance),
    m_gain(gain),
    m_parport(parport)
{
}

const PS4000A_CHANNEL& Channel::channelType() const
{
    return m_channelType;
}

const std::string& Channel::hppdl() const
{
return m_hppdl;
}

const PS4000A_COUPLING& Channel::coupling() const
{
    return m_coupling;
}

const PS4000A_RANGE& Channel::range() const
{
    return m_range;
}

int Channel::rangeInt() const
{
    return rangeToMilliVolts(m_range);
}

const bool& Channel::isEnabled() const
{
    return m_enabled;
}

const double& Channel::analogOffset() const
{
    return m_analogOffset;
}

const double& Channel::resistance() const
{
    return m_resistance;
}

const double& Channel::gain() const
{
    return m_gain;
}

const bool& Channel::isParport() const
{
    return m_parport;
}

std::string Channel::channelTypeName() const
{
    switch (m_channelType) {
        case PS4000A_CHANNEL_A:
            return "CHANNEL_A";
        case PS4000A_CHANNEL_B:
            return "CHANNEL_B";
        case PS4000A_CHANNEL_C:
            return "CHANNEL_C";
        case PS4000A_CHANNEL_D:
            return "CHANNEL_D";
        case PS4000A_CHANNEL_E:
            return "CHANNEL_E";
        case PS4000A_CHANNEL_F:
            return "CHANNEL_F";
        case PS4000A_CHANNEL_G:
            return "CHANNEL_G";
        case PS4000A_CHANNEL_H:
            return "CHANNEL_H";
        case PS4000A_EXTERNAL:
            return "EXTERNAL";
        case PS4000A_TRIGGER_AUX:
            return "TRIGGER_AUX";
        case PS4000A_MAX_TRIGGER_SOURCES:
            return "MAX_TRIGGER_SOURCES";
        case PS4000A_PULSE_WIDTH_SOURCE:
            return "PULSE_WIDTH_SOURCE";
        default:
            return "INVALID_CHANNEL";
    }
}

} // namespace ps4000a
