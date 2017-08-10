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

#ifndef MEASUREMENTDATA_H_INCLUDED
#define MEASUREMENTDATA_H_INCLUDED

#include <ps4000aApi.h>
#include <map>
#include <string>
#include <vector>
/**
 * Namespace for the PicoScope implementation
 */
namespace ps4000a {

/**
 * A class that contains data, provided by the measurement.
 */
class MeasurementData {
    double m_energy; ///< Capability of energy consumption measurement (in Joules)
    double m_minimumPower; ///< Capability of measuring minimum power dissipation (in Watts)
    double m_maximumPower; //< Capability of measuring maximum power dissipation (in Watts)
    double m_elapsedTime;  ///< Capability of measuring time spent (in seconds)

public:
    MeasurementData();
    const double& energy() const;
    const double& minPower() const;
    const double& maxPower() const;
    const double& elapsedTime() const;

    void setEnergy(double energy);
    void setMinPower(double minPower);
    void setMaxPower(double maxPower);
    void setElapsedTime(double elapsedTime);

    void gainEnergy(double energy);
    void gainElapsedTime(double elapsedTime);

};

typedef std::pair<PS4000A_CHANNEL, std::string> MeasuredChannel;

/**
 * A mapping from HPP-DL component ids and scope channels to sets of measurement data.
 */
typedef std::map<MeasuredChannel, MeasurementData> MeasurementMap;

/**
 * Store MeasurementMap and raw data
 */
typedef std::pair<MeasurementMap, std::string> MeasuredValues;

/**
 * A list from the MeasuredValues
 */
typedef std::vector<MeasuredValues> MeasuredValuesList;

} // namespace ps4000a

#endif // MEASUREMENTDATA_H_INCLUDED
