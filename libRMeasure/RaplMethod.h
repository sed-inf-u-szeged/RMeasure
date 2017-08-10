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

#ifndef RAPL_H_INCLUDED
#define RAPL_H_INCLUDED

#include "Method.h"

namespace repara {
namespace measurement {

/**
 * Namespace for the RAPL-based measurement method implementation.
 */
namespace rapl {

/**
 * A simple measurement method implementation relying on RAPL energy readings.
 * The /dev/cpu/?/msr driver must be enabled and permissions set
 * to allow read access for this to work.
 */
class RaplMeasurement : public Measurement {
    bool _inProgress; ///< specifies whether the measurement is in progress
    KernelSourceMap _kernelResults; ///< contains the results of the measurement for each kernel

public:
    RaplMeasurement();
    ~RaplMeasurement();

    void stop();
    const KernelSourceMap& kernelSourceMap() const;
    const SourceMap aggregatedSources(const std::string& kernelName) const;
    const SourceContainer kernelSources(const std::string& kernelName) const;

    const bool& isInProgress() const;

}; // class RaplMethod::RaplMeasurement

class RaplMethod : public Method {
    SourceCapabilityMap _caps; ///< contains sourceCapabilities
    static RaplMethod* _instance;

    RaplMethod();
    ~RaplMethod();
    RaplMethod(const RaplMethod&) = delete;
    void operator=(const RaplMethod&)  = delete;

public:
    static RaplMethod* getInstance();
    static void deleteInstance();

    /**
     * This function will return the available sourceCapabilities.
     */
    const SourceCapabilityMap& sourceCapabilities() const;

    /**
     * Start a measurement. Calling stop() on the
     * returned Measurement object will retrieve the results from the
     * registers, and store this data into the SourceMap.
     */
    Measurement* start();


}; // class RaplMethod

} // namespace repara::measurement::rapl
} // namespace repara::measurement
} // namespace repara

#endif // TIMERMETHOD_H_INCLUDED
