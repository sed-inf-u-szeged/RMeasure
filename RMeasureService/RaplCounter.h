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

#ifndef RAPLCOUNTER_H_INCLUDED
#define RAPLCOUNTER_H_INCLUDED

#include <map>
#include <vector>
#include <stdint.h> /* for uint64 definition */
#include <time.h>   /* for clock_gettime */

#define BILLION 1000000000L

#define MSR_RAPL_POWER_UNIT     0x606
/* Package RAPL Domain */
#define MSR_PKG_ENERGY_STATUS   0x611

/**
 * Namespace for the Rapl implementation
 */
namespace rapl {

class MeasurementData {
    double m_lastPackageEnergyRaw;
    timespec m_lastTime;
    double m_calculatedPackageEnergy;
    uint64_t m_calculatedElapsedTime;

public:
    MeasurementData();
    void gainCapabilites(const long long& pkgEnergyStatus, const double& energyUnits, const timespec& currentTime);
    void setLastTime(const timespec& time);
    void setLastPackageEnergy(const double& energy);
    const double& packageEnergy() const;
    const uint64_t& elapsedTime() const;
};

typedef std::pair<std::string, int> Processor;
typedef std::map<Processor, MeasurementData> MeasurementMap;
typedef std::vector<MeasurementMap> KernelList;

class RaplCounter {
    KernelList m_kernelList;
    std::vector<Processor> m_processors;

    int openMSR(int core);
    long long readMSR(int fd, int which);

public:
    RaplCounter(std::vector<Processor> processors);
    ~RaplCounter();

    const KernelList& kernelList() const;
    const std::vector<Processor>& processors() const;

    void calculate(bool isBegin = false);
    void startMeasurement();
};

} // namespace rapl

#endif // RAPLCOUNTER_H_INCLUDED
