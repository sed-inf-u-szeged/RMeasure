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

#include "MeasurementData.h"

/**
 * Namespace for the PicoScope implementation
 */
namespace ps4000a {

MeasurementData::MeasurementData() :
    m_energy(0.0),
    m_minimumPower(-1.0),
    m_maximumPower(0.0),
    m_elapsedTime(0.0)
{
}

const double& MeasurementData::energy() const
{
    return m_energy;
}

const double& MeasurementData::minPower() const
{
    return m_minimumPower;
}

const double& MeasurementData::maxPower() const
{
    return m_maximumPower;
}

const double& MeasurementData::elapsedTime() const
{
    return m_elapsedTime;
}

void MeasurementData::setEnergy(double energy)
{
    m_energy = energy;
}

void MeasurementData::setMinPower(double minPower)
{
    m_minimumPower = minPower;
}

void MeasurementData::setMaxPower(double maxPower)
{
    m_maximumPower = maxPower;
}

void MeasurementData::setElapsedTime(double elapsedTime)
{
    m_elapsedTime = elapsedTime;
}

void MeasurementData::gainEnergy(double energy)
{
    m_energy += energy;
}

void MeasurementData::gainElapsedTime(double elapsedTime)
{
    m_elapsedTime += elapsedTime;
}

} // namespace ps4000a
