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

#ifndef PICOSCOPEMETHOD_H_INCLUDED
#define PICOSCOPEMETHOD_H_INCLUDED

#include "Method.h"
#include "PicoScopeModel.h"

namespace repara {
namespace measurement {

/**
 * Namespace for oscilloscope-based measurement method implementation(s).
 */
namespace scope {

/*
 * Time units in which time is measured.
*/
enum TimeUnit
{
    TIME_S,
    TIME_MS,
    TIME_US,
    TIME_NS,
    TIME_PS,
    TIME_FS
};

/**
 * A PicoScope-based measurement method implementation.
 *
 * Tested models: 5203, 4824
 *
 */
class PicoScopeMeasurement : public Measurement {

    /**
     * A mapping from the measured kernels to list of measured raw data.
     */
    typedef std::map<std::string, std::vector<std::string> > RawDataMap;

    RawDataMap _rawData; ///< contains the raw data of the measured kernels
    bool _allowRaw; ///< specifies whether collecting raw data is enabled
    bool _inProgress; ///< specifies whether measurement is in progress
    KernelSourceMap _kernelResults; ///< contains the results for each measurement


public:
    PicoScopeMeasurement();
    ~PicoScopeMeasurement();
    void stop();

    const KernelSourceMap& kernelSourceMap() const;
    const SourceMap aggregatedSources(const std::string& kernelName) const;
    const SourceContainer kernelSources(const std::string& kernelName) const;

    /**
     * \brief Provide the raw data list of the given kernel.
     * Each data is the digital data converted from analog signals (ADC value) in milliVolts.
     * \return with the raw data (if exist, else return with an empty vector)
    */
    const std::vector<std::string> rawData(const std::string& kernelName) const;

    /**
     * \brief Allow collecting raw data.
     * If it is disabled, then rawData() will return with an empty string.
     */
    void allowRaw(const bool isAllowed);

    /**
     * Ensure information about whether raw Data is allowed.
     */
    const bool& isRawAllowed() const;

    /**
     * Ensure information about whether the measurement is in progress.
     */
    const bool& isInProgress() const;

}; // class PicoScopeMethod::PicoScopeMeasurement

class PicoScopeMethod : public Method {
    SourceCapabilityMap _caps; ///< contains sourceCapabilities
    PicoScopeModel _model; ///< contains model information
    bool _isAvailable; ///< specifies whether the scope is available for the measurement
    static PicoScopeMethod* _instance;

    PicoScopeMethod();
    ~PicoScopeMethod();
    PicoScopeMethod(const PicoScopeMethod&) = delete;
    void operator=(const PicoScopeMethod&)  = delete;

    /**
     * \brief Open the scope device.
     * \return true if the opening was successful, false otherwise
     */
    bool openScope();

     /**
     * \brief Close the scope device.
     * \return true if the closing was successful, false otherwise
     */
    bool closeScope();

public:
    static PicoScopeMethod* getInstance();
    static void deleteInstance();

    /**
     * This function will return the available sourceCapabilities.
     */
    const SourceCapabilityMap& sourceCapabilities() const;

    /**
     * This function will return the used PicoScopeModel information.
     */
    const PicoScopeModel& scopeModel() const;

    /**
     * \brief Start a measurement.
     * This will start the streaming mode. Calling
     * stop() on the returned Measurement object will retrieve the collected values
     * of the streaming mode, and these results will go into the SourceMap.
     * If the _caps map which contains sourceCapabilities is empty, this will return NULL.
     * \return the started measurement object
     */
    Measurement* start();

    /*
    * This function sets the ammount of the raw data samples to be collected.
    * This means, we will receive data in each sampleInterval TimeUnit
    * (eg. each 4 milliseconds, where 4 is the sampleInterval,
    * and millisecond is the TimeUnit). Default value is 1 millisecond.
    * \return true if setting the sample rate was successful, false otherwise
    */
    bool setSampleRate(const int& sampleInterval, const TimeUnit& sampleTime) const;

    /**
     * Ensure information about whether the oscilloscope is available.
     */
    const bool& isAvailable() const;

}; // class PicoScopeMethod

} // namespace repara::measurement::scope
} // namespace repara::measurement
} // namespace repara

#endif // PICOSCOPEMETHOD_H_INCLUDED
