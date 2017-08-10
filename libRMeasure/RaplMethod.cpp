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

#include "RaplMethod.h"

#include <xmlrpc-c/client_simple.hpp>

namespace repara {
namespace measurement {
namespace rapl {

const std::string startListeningCommand = "rapl.startListening";
const std::string stopListeningCommand = "rapl.stopListening";
const std::string getMeasuredDataCommand = "rapl.getMeasuredData";
const std::string getMeasuredProcessorsCommand = "rapl.getMeasuredProcessors";
const std::string getMeasuredKernelsCommand = "rmeasure.getMeasuredKernels";

RaplMeasurement::RaplMeasurement()
    : _inProgress(true), _kernelResults()
{
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value startListeningResult;
    myClient.call(getenv(RMEASURESERVICE), startListeningCommand, "", &startListeningResult);
    bool startListening = static_cast<bool>(xmlrpc_c::value_boolean(startListeningResult));
    if (!startListening)
        _inProgress = false;
}

RaplMeasurement::~RaplMeasurement()
{
}

void RaplMeasurement::stop()
{
    if (_inProgress) {
        xmlrpc_c::clientSimple myClient;
        xmlrpc_c::value stopListeningResult;

        myClient.call(getenv(RMEASURESERVICE), stopListeningCommand, "", &stopListeningResult);

        bool stopListening = static_cast<bool>(xmlrpc_c::value_boolean(stopListeningResult));
        if (stopListening) {
            xmlrpc_c::value measurementResults, kernelNames;
            myClient.call(getenv(RMEASURESERVICE), getMeasuredDataCommand, "", &measurementResults);
            myClient.call(getenv(RMEASURESERVICE), getMeasuredKernelsCommand, "", &kernelNames);

            std::vector<xmlrpc_c::value> kernels = xmlrpc_c::value_array(kernelNames).cvalue();
            std::vector<xmlrpc_c::value> kernelResults = xmlrpc_c::value_array(measurementResults).cvalue();

            if (kernels.size() == kernelResults.size()) {
                std::vector<xmlrpc_c::value>::iterator resultsIt = kernelResults.begin();
                std::vector<xmlrpc_c::value>::iterator kernelsIt = kernels.begin();
                for (; resultsIt != kernelResults.end(); ++resultsIt, ++kernelsIt) {
                    const xmlrpc_c::value_struct measurements = static_cast<xmlrpc_c::value_struct>(*resultsIt);
                    std::map<std::string, xmlrpc_c::value> measurementsMap(static_cast<std::map<std::string, xmlrpc_c::value> >(measurements));
                    std::map<std::string, xmlrpc_c::value>::iterator measurementsIt = measurementsMap.begin();
                    SourceMap result;
                    for (; measurementsIt != measurementsMap.end(); ++measurementsIt) {
                        const std::string device = static_cast<std::string>(xmlrpc_c::value_string(measurementsIt->first));
                        const xmlrpc_c::value_struct results = static_cast<xmlrpc_c::value_struct>(measurementsIt->second);
                        std::map<std::string, xmlrpc_c::value> resultsMap(static_cast<std::map<std::string, xmlrpc_c::value> >(results));
                        result[device][SourceCapability::Energy] = static_cast<double>(xmlrpc_c::value_double(resultsMap["energy"]));
                        result[device][SourceCapability::ElapsedTime] = static_cast<double>(xmlrpc_c::value_double(resultsMap["elapsedTime"]));
                        result[device][SourceCapability::AveragePower] = result[device][SourceCapability::Energy] / result[device][SourceCapability::ElapsedTime];
                    }
                    const std::string kernelName = static_cast<std::string>(xmlrpc_c::value_string(*kernelsIt));
                    _kernelResults[kernelName].push_back(result);
                }
            }
        }
    }
}

const Measurement::KernelSourceMap& RaplMeasurement::kernelSourceMap() const
{
    return _kernelResults;
}

const Measurement::SourceMap RaplMeasurement::aggregatedSources(const std::string& kernelName) const
{
    SourceMap aggregatedSources;
    KernelSourceMap::const_iterator kernelIt = _kernelResults.find(kernelName);
    if (kernelIt != _kernelResults.end()) {
        SourceContainer::const_iterator containerIt = kernelIt->second.begin();
        for (; containerIt != kernelIt->second.end(); ++containerIt) {
            SourceMap::const_iterator sourceIt = containerIt->begin();
            for (; sourceIt != containerIt->end(); ++sourceIt) {
                const std::string& device = sourceIt->first;
                const DataMap& dataMap = sourceIt->second;
                DataMap::const_iterator dataMapIt;

                dataMapIt = dataMap.find(SourceCapability::Energy);
                aggregatedSources[device][SourceCapability::Energy] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;

                dataMapIt = dataMap.find(SourceCapability::ElapsedTime);
                aggregatedSources[device][SourceCapability::ElapsedTime] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;

                dataMapIt = dataMap.find(SourceCapability::AveragePower);
                aggregatedSources[device][SourceCapability::AveragePower] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;
            }
        }
    }
    return aggregatedSources;
}

const Measurement::SourceContainer RaplMeasurement::kernelSources(const std::string& kernelName) const
{
    SourceContainer sources;
    KernelSourceMap::const_iterator kernelIt = _kernelResults.find(kernelName);
    if (kernelIt != _kernelResults.end())
        sources = kernelIt->second;

    return sources;
}

const bool& RaplMeasurement::isInProgress() const
{
    return _inProgress;
}

RaplMethod::RaplMethod()
    : _caps()
{
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value measuredProcessors;

    myClient.call(getenv(RMEASURESERVICE), getMeasuredProcessorsCommand, "", &measuredProcessors);
    std::vector<xmlrpc_c::value> processors = xmlrpc_c::value_array(measuredProcessors).cvalue();

    std::vector<xmlrpc_c::value>::iterator procIt = processors.begin();
    for (; procIt != processors.end(); ++procIt) {
        const std::string device = static_cast<std::string>(xmlrpc_c::value_string(*procIt));
        _caps[device] = SourceCapability::ElapsedTime;
        _caps[device] |= SourceCapability::Energy;
        _caps[device] |= SourceCapability::AveragePower;
    }
}

RaplMethod::~RaplMethod()
{
}

RaplMethod* RaplMethod::_instance = NULL;

RaplMethod* RaplMethod::getInstance()
{
    if (!_instance)
        _instance = new RaplMethod;
    return _instance;
}

void RaplMethod::deleteInstance()
{
    delete _instance;
    _instance = NULL;
}

const Method::SourceCapabilityMap& RaplMethod::sourceCapabilities() const
{
    return _caps;
}

Measurement* RaplMethod::start()
{
    if (_caps.empty())
        return NULL;
    return new RaplMeasurement();
}

} // namespace repara::measurement::rapl
} // namespace repara::measurement
} // namespace repara
