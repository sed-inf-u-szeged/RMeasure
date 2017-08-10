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

#include "PicoScopeMethod.h"

#include <xmlrpc-c/client_simple.hpp>

#define XML_SIZE_LIMIT 64*1024*1024 // 64 MB

namespace repara {
namespace measurement {
namespace scope {

/** commands which are used to communicate with the scope model via the ScopeControlService*/
const std::string startStreamingCommand = "pico.startStreaming";
const std::string stopStreamingCommand = "pico.stopStreaming";
const std::string getValuesCommand = "pico.getValues";
const std::string openScopeCommand = "pico.open";
const std::string closeScopeCommand = "pico.close";
const std::string scopeInfoCommand = "pico.getScopeInfo";
const std::string channelInfoCommand = "pico.channelInfo";
const std::string getRawDataCommand = "pico.rawData";
const std::string setSampleCommand = "pico.setSample";

/** commands which are used to communicate with RMeasure Server via the RMeasureService */
const std::string startListeningCommand = "scope.startListening";
const std::string stopListeningCommand = "scope.stopListening";
const std::string getMeasuredKernelsCommand = "rmeasure.getMeasuredKernels";

PicoScopeMeasurement::PicoScopeMeasurement()
    : _rawData(), _allowRaw(false), _inProgress(true), _kernelResults()
{
    xmlrpc_c::clientSimple myClient;

    // start listening kernels by the RMeasureService
    xmlrpc_c::value startListeningResult;
    myClient.call(getenv(RMEASURESERVICE), startListeningCommand, "", &startListeningResult);
    bool startListening = static_cast<bool>(xmlrpc_c::value_boolean(startListeningResult));

    if (startListening) {

        // start the streaming via ScopeControlService
        xmlrpc_c::value startStreamingResult;
        myClient.call(getenv(SCOPESERVICE), startStreamingCommand, "", &startStreamingResult);
        bool startStreaming = static_cast<bool>(xmlrpc_c::value_boolean(startStreamingResult));

        // ScopeControlService is failed to start the streaming, needs to stop the listening
        if (!startStreaming) {
            xmlrpc_c::value stopListeningResult;
            myClient.call(getenv(SCOPESERVICE), stopListeningCommand, "", &stopListeningResult);
            _inProgress = false;
        }
    }
    else {
        _inProgress = false;
    }
}

PicoScopeMeasurement::~PicoScopeMeasurement()
{
}

void PicoScopeMeasurement::stop()
{
    if (_inProgress) {
        xmlrpc_c::value stopStreamingResult, stopListeningResult;
        xmlrpc_c::clientSimple myClient;
        myClient.call(getenv(SCOPESERVICE), stopStreamingCommand, "", &stopStreamingResult);

        myClient.call(getenv(RMEASURESERVICE), stopListeningCommand, "", &stopListeningResult);

        bool stopStreaming = static_cast<bool>(xmlrpc_c::value_boolean(stopStreamingResult));
        bool stopListening = static_cast<bool>(xmlrpc_c::value_boolean(stopListeningResult));

        if (stopStreaming && stopListening) {
            xmlrpc_c::value measurementResults, kernelNames;
            myClient.call(getenv(SCOPESERVICE), getValuesCommand, "", &measurementResults);
            myClient.call(getenv(RMEASURESERVICE), getMeasuredKernelsCommand, "", &kernelNames);

            std::vector<xmlrpc_c::value> kernels = xmlrpc_c::value_array(kernelNames).cvalue();
            std::vector<xmlrpc_c::value> kernelResults = xmlrpc_c::value_array(measurementResults).cvalue();

            if (kernels.size() == kernelResults.size()) {
                std::vector<xmlrpc_c::value>::iterator resultsIt = kernelResults.begin();
                std::vector<xmlrpc_c::value>::iterator kernelsIt = kernels.begin();


                xmlrpc_c::value rawDataResult; // use this if collecting raw is allowed
                std::vector<xmlrpc_c::value> rawData; // use this if collecting raw is allowed
                std::vector<xmlrpc_c::value>::iterator rawIt; // use this if collecting raw is allowed

                if (_allowRaw) {
                    // set XML_SIZE limit to be able to receive larger raw data
                    xmlrpc_limit_set(XMLRPC_XML_SIZE_LIMIT_ID, XML_SIZE_LIMIT);
                    myClient.call(getenv(SCOPESERVICE), getRawDataCommand, "", &rawDataResult);
                    rawData = xmlrpc_c::value_array(rawDataResult).cvalue();
                    if (rawData.size() != kernels.size())
                        _allowRaw = false;
                    else
                        rawIt = rawData.begin();
                }

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
                        result[device][SourceCapability::MinimumPower] = static_cast<double>(xmlrpc_c::value_double(resultsMap["minPower"]));
                        result[device][SourceCapability::MaximumPower] = static_cast<double>(xmlrpc_c::value_double(resultsMap["maxPower"]));
                        result[device][SourceCapability::ElapsedTime] = static_cast<double>(xmlrpc_c::value_double(resultsMap["elapsedTime"]));
                        result[device][SourceCapability::AveragePower] = result[device][SourceCapability::Energy] / result[device][SourceCapability::ElapsedTime];
                    }
                    const std::string kernelName = static_cast<std::string>(xmlrpc_c::value_string(*kernelsIt));

                    _kernelResults[kernelName].push_back(result);

                    if (_allowRaw) {
                        _rawData[kernelName].push_back(static_cast<std::string>(xmlrpc_c::value_string(*rawIt)));
                        ++rawIt;
                    }

                }
            }
        }
    }
}

const Measurement::KernelSourceMap& PicoScopeMeasurement::kernelSourceMap() const
{
    return _kernelResults;
}

const Measurement::SourceMap PicoScopeMeasurement::aggregatedSources(const std::string& kernelName) const
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

                dataMapIt = dataMap.find(SourceCapability::MinimumPower);
                aggregatedSources[device][SourceCapability::MinimumPower] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;

                dataMapIt = dataMap.find(SourceCapability::MaximumPower);
                aggregatedSources[device][SourceCapability::MaximumPower] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;

                dataMapIt = dataMap.find(SourceCapability::ElapsedTime);
                aggregatedSources[device][SourceCapability::ElapsedTime] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;

                dataMapIt = dataMap.find(SourceCapability::AveragePower);
                aggregatedSources[device][SourceCapability::AveragePower] += (dataMapIt != dataMap.end()) ? dataMapIt->second : 0.0;
            }
        }
    }
    return aggregatedSources;
}

const Measurement::SourceContainer PicoScopeMeasurement::kernelSources(const std::string& kernelName) const
{
    SourceContainer sources;
    KernelSourceMap::const_iterator kernelIt = _kernelResults.find(kernelName);
    if (kernelIt != _kernelResults.end())
        sources = kernelIt->second;

    return sources;
}

const std::vector<std::string> PicoScopeMeasurement::rawData(const std::string& kernelName) const
{
    RawDataMap::const_iterator rawIt = _rawData.find(kernelName);
    if (rawIt != _rawData.end())
        return rawIt->second;
    else {
        std::vector<std::string> emptyList;
        return emptyList;
    }
}

void PicoScopeMeasurement::allowRaw(const bool isAllowed)
{
    _allowRaw = isAllowed;
}

const bool& PicoScopeMeasurement::isRawAllowed() const
{
    return _allowRaw;
}

const bool& PicoScopeMeasurement::isInProgress() const
{
    return _inProgress;
}

PicoScopeMethod* PicoScopeMethod::_instance = NULL;

PicoScopeMethod* PicoScopeMethod::getInstance()
{
    if (!_instance)
        _instance = new PicoScopeMethod;
    return _instance;
}

void PicoScopeMethod::deleteInstance()
{
    delete _instance;
    _instance = NULL;
}

PicoScopeMethod::PicoScopeMethod()
    : _caps(), _model(), _isAvailable(false)
{
    _isAvailable = openScope();
}

const Method::SourceCapabilityMap& PicoScopeMethod::sourceCapabilities() const
{
    return _caps;
}

const PicoScopeModel& PicoScopeMethod::scopeModel() const
{
    return _model;
}

Measurement* PicoScopeMethod::start()
{
    if (_caps.empty() || !_isAvailable)
        return NULL;

    return new PicoScopeMeasurement();
}

bool PicoScopeMethod::openScope()
{
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value openResult, versionInformation, channelsInformation;

    myClient.call(getenv(SCOPESERVICE), openScopeCommand, "", &openResult);
    bool result = xmlrpc_c::value_boolean(openResult);
    if (result) {
        myClient.call(getenv(SCOPESERVICE), scopeInfoCommand, "", &versionInformation);
        _model.setVersions(versionInformation);
        myClient.call(getenv(SCOPESERVICE), channelInfoCommand, "", &channelsInformation);
        _model.fillChannels(channelsInformation);

        ChannelVector::const_iterator channelsIt = _model.channels().begin();
        for (; channelsIt != _model.channels().end(); ++channelsIt) {
            if (channelsIt->isEnabled() && !channelsIt->isParport()) {
                const std::string device = channelsIt->hppdl();
                _caps[device] = SourceCapability::ElapsedTime;
                _caps[device] |= SourceCapability::Energy;
                _caps[device] |= SourceCapability::MinimumPower;
                _caps[device] |= SourceCapability::MaximumPower;
                _caps[device] |= SourceCapability::AveragePower;
            }
        }
    }
    return result;
}

bool PicoScopeMethod::closeScope()
{
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value resultMsg;
    myClient.call(getenv(SCOPESERVICE), closeScopeCommand, "", &resultMsg);
    return xmlrpc_c::value_boolean(resultMsg);
}


PicoScopeMethod::~PicoScopeMethod()
{
    closeScope();
}

bool PicoScopeMethod::setSampleRate(const int& sampleInterval, const TimeUnit& sampleTime) const
{
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value resultMsg;
    std::string sampleUnit;
    switch (sampleTime) {
        case TIME_S :
            sampleUnit = "TIME_S";
            break;
        case TIME_MS :
            sampleUnit = "TIME_MS";
            break;
        case TIME_US :
            sampleUnit = "TIME_US";
            break;
        case TIME_NS :
            sampleUnit = "TIME_NS";
            break;
        case TIME_PS :
            sampleUnit = "TIME_PS";
            break;
        case TIME_FS :
            sampleUnit = "TIME_FS";
            break;
        default :
            sampleUnit = "TIME_MS";
            break;
    }

    myClient.call(getenv(SCOPESERVICE), setSampleCommand, "is", &resultMsg, sampleInterval, sampleUnit.c_str());
    return xmlrpc_c::value_boolean(resultMsg);
}

const bool& PicoScopeMethod::isAvailable() const
{
    return _isAvailable;
}

} // namespace repara::measurement::scope
} // namespace repara::measurement
} // namespace repara
