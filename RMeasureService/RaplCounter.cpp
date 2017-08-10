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

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "RaplCounter.h"

namespace rapl {

MeasurementData::MeasurementData() :
    m_lastPackageEnergyRaw(0.0),
    m_lastTime(),
    m_calculatedPackageEnergy(0.0),
    m_calculatedElapsedTime(0)
{
}

uint64_t diff_ms(timespec end, timespec start)
{
    // return in nanosec
    return BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

}

void MeasurementData::gainCapabilites(const long long& pkgEnergyStatus, const double& energyUnits, const timespec& currentTime)
{
    double packageEnergyRaw = (double)pkgEnergyStatus*energyUnits;
    if (m_lastPackageEnergyRaw > packageEnergyRaw)
        m_calculatedPackageEnergy += (((double)(pow(2,32)-1)*energyUnits)-m_lastPackageEnergyRaw)+packageEnergyRaw;
    else
        m_calculatedPackageEnergy += packageEnergyRaw - m_lastPackageEnergyRaw;

    m_calculatedElapsedTime += diff_ms(currentTime, m_lastTime);
}

const double& MeasurementData::packageEnergy() const
{
    return m_calculatedPackageEnergy;
}

const uint64_t& MeasurementData::elapsedTime() const
{
    return m_calculatedElapsedTime;
}

RaplCounter::RaplCounter(std::vector<Processor> processors) :
    m_kernelList(),
    m_processors(processors)
{
}

void MeasurementData::setLastTime(const timespec& time)
{
    m_lastTime = time;
}

void MeasurementData::setLastPackageEnergy(const double& energy)
{
    m_lastPackageEnergyRaw = energy;
}

void RaplCounter::calculate(bool isBegin)
{
    if (isBegin) {
        MeasurementMap measurements;
        std::vector<Processor>::iterator procIt = m_processors.begin();
        for (; procIt != m_processors.end(); ++procIt) {
            MeasurementData measurementData;
            measurements.insert(std::pair<Processor, MeasurementData>(*procIt, measurementData));
        }
        m_kernelList.push_back(measurements);
    }

    MeasurementMap::iterator measurementIt = m_kernelList.back().begin();
    for (; measurementIt != m_kernelList.back().end(); ++measurementIt) {
        int core = measurementIt->first.second;
        long long msrResult = readMSR(core, MSR_RAPL_POWER_UNIT);
        double energyUnits = pow(0.5,(double)((msrResult>>8)&0x1f));
        msrResult = readMSR(core, MSR_PKG_ENERGY_STATUS);

        timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);  /* mark start time */

        if (!isBegin)
            measurementIt->second.gainCapabilites(msrResult, energyUnits, currentTime);

        measurementIt->second.setLastPackageEnergy((double)msrResult*energyUnits);
        measurementIt->second.setLastTime(currentTime);
    }
}

void RaplCounter::startMeasurement()
{
    m_kernelList.clear();
}

const KernelList& RaplCounter::kernelList() const
{
    return m_kernelList;
}

const std::vector<Processor>& RaplCounter::processors() const
{
    return m_processors;
}

RaplCounter::~RaplCounter()
{
}

int RaplCounter::openMSR(int core)
{
    char msr_filename[BUFSIZ];
    int fd = -1;

    sprintf(msr_filename, "/dev/cpu/%d/msr", core);
    fd = open(msr_filename, O_RDONLY);
    if ( fd < 0 ) {
        if ( errno == ENXIO ) {
              fprintf(stderr, "rdmsr: No CPU %d\n", core);
        } else if ( errno == EIO ) {
              fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
        } else {
              perror("rdmsr:open");
              fprintf(stderr,"Trying to open %s\n",msr_filename);
        }
    }

    return fd;
}


long long RaplCounter::readMSR(int core, int which)
{
    uint64_t data = 0;
    int fd = openMSR(core);
    if (fd > -1)
    {
        if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
            perror("rdmsr:pread");
            close(fd);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    return (long long)data;
}

} // namespace rapl
