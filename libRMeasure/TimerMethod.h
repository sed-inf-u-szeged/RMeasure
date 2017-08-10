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

#ifndef TIMERMETHOD_H_INCLUDED
#define TIMERMETHOD_H_INCLUDED

#include "Method.h"

namespace repara {
namespace measurement {

/**
 * Namespace for the time()-based measurement method implementation.
 */
namespace timer {

/**
 * A simple measurement method implementation relying on the time() function.
 * The only supported type of measured data is wall-clock time elapsed between
 * TimerMethod::start() and TimerMeasurement::stop(), and it's reported as a measurement done on the whole
 * system.
 */
class TimerMeasurement : public Measurement {
    bool _inProgress; ///< specifies whether the measurement is in progress
    KernelSourceMap _kernelResults;

public:
    TimerMeasurement();
    ~TimerMeasurement();
    void stop();

    const KernelSourceMap& kernelSourceMap() const;
    const SourceMap aggregatedSources(const std::string& kernelName) const;
    const SourceContainer kernelSources(const std::string& kernelName) const;
    const bool& isInProgress() const;

}; // class TimerMethod::TimerMeasurement

class TimerMethod : public Method {
    SourceCapabilityMap _caps;
    static TimerMethod* _instance;
    TimerMethod();
    ~TimerMethod();

    TimerMethod(const TimerMethod&) = delete;
    void operator=(const TimerMethod&)  = delete;

public:
    static TimerMethod* getInstance();
    static void deleteInstance();

    const SourceCapabilityMap& sourceCapabilities() const;

    /**
     * Start a measurement, i.e., store the timestamp of the call. Calling
     * stop() on the returned Measurement object will retrieve another timestamp
     * and their difference will go into the SourceMap.
     */
    Measurement* start();

}; // class TimerMethod

} // namespace repara::measurement::timer
} // namespace repara::measurement
} // namespace repara

#endif // TIMERMETHOD_H_INCLUDED
