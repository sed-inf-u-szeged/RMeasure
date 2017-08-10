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

#include "DataModel.h"

#include <Method.h>
#include <PicoScopeMethod.h>
#include <RaplMethod.h>
#include <TimerMethod.h>
#include <iostream>

using namespace datamodel;

using namespace repara::measurement;
using namespace repara::measurement::scope;
using namespace repara::measurement::rapl;
using namespace repara::measurement::timer;


void printUsage()
{
    std::cout << "Usage:\t ./measureTool --config configFile [--scope --rapl --timer --isAggregate]" << std::endl;
}

const std::string convertCapability(const SourceCapability& sourceCapability)
{
    if (sourceCapability == SourceCapability::Energy)
        return "ENERGY";
    if (sourceCapability == SourceCapability::MinimumPower)
        return "MINIMUMPOWER";
    if (sourceCapability == SourceCapability::AveragePower)
        return "AVERAGEPOWER";
    if (sourceCapability == SourceCapability::MaximumPower)
        return "MAXIMUMPOWER";
    if (sourceCapability == SourceCapability::ElapsedTime)
        return "ELAPSEDTIME";
    if (sourceCapability == SourceCapability::KernelTime)
        return "KERNELTIME";
    if (sourceCapability == SourceCapability::UserTime)
        return "USERTIME";

    return "";
}

const std::string convertSourceMap(const Measurement::SourceMap& sourceMap, const std::string& type)
{
    std::string result;
    Measurement::SourceMap::const_iterator sourceIt = sourceMap.begin();
    for (; sourceIt != sourceMap.end(); ++sourceIt) {
        result.append("\tdevice: " + sourceIt->first + "\n");
        const Measurement::DataMap& dataMap = sourceIt->second;
        Measurement::DataMap::const_iterator dataIt = dataMap.begin();
        for (; dataIt != dataMap.end(); ++dataIt)
            result.append("\tmetric: " + type + "_" +convertCapability(dataIt->first) + " " +  std::to_string(dataIt->second) + "\n");
    }
    result.append("\t---------------------------------------------\n");
    return result;
}

const std::string convertKernelSourceMap(const Measurement& measurement, const std::string& type, bool isAggregate)
{
    std::string result;

    Measurement::KernelSourceMap::const_iterator kernelSourceIt = measurement.kernelSourceMap().begin();
    for (; kernelSourceIt != measurement.kernelSourceMap().end(); ++kernelSourceIt) {
        const std::string& kernelName = kernelSourceIt->first;
        result.append("kernelName: " + kernelName + "\n");
        if (isAggregate) {
            result.append(convertSourceMap(measurement.aggregatedSources(kernelName), type));
        }
        else {
            Measurement::SourceContainer::const_iterator resultsIt = kernelSourceIt->second.begin();
            for (; resultsIt != kernelSourceIt->second.end(); ++resultsIt) {
                result.append(convertSourceMap(*resultsIt, type));
            }
        }
    }

    return result;
}

int main(int argc, char *argv[]) {

    std::string configName;
    #ifdef RAPL
    bool isRaplEnabled = false;
    #endif
    #ifdef TIMER
    bool isTimerEnabled = false;
    #endif
    #ifdef SCOPE
    bool isScopeEnabled = false;
    #endif

    bool isAggregate = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg.compare("-c")) == 0 || (arg.compare("--config")) == 0) {
            if (i + 1 < argc) {
                configName = argv[++i];
            } else {
                std::cerr << "--config option requires one argument." << std::endl;
                return EXIT_FAILURE;
            }
        }
        #ifdef SCOPE
        if ((arg.compare("--scope")) == 0) {
            isScopeEnabled = true;
        }
        #endif
        #ifdef RAPL
        if ((arg.compare("--rapl")) == 0) {
            isRaplEnabled = true;
        }
        #endif
        #ifdef TIMER
        if ((arg.compare("--timer")) == 0) {
            isTimerEnabled = true;
        }
        #endif
        if ((arg.compare("--isAggregate")) == 0) {
            isAggregate = true;
        }
        if ((arg.compare("--help")) == 0) {
            printUsage();
            return EXIT_SUCCESS;
        }
    }

    if (configName.empty()) {
        std::cerr << "Need to set a config file!" << std::endl;
        printUsage();
        return EXIT_FAILURE;
    }

    DataModel dataModel(configName);

    #ifdef RAPL
    RaplMethod* raplMethod = NULL;
    if (isRaplEnabled) {
        raplMethod = RaplMethod::getInstance();
    }
    #endif

    #ifdef SCOPE
    PicoScopeMethod* scopeMethod = NULL;
    if (isScopeEnabled) {
        scopeMethod = PicoScopeMethod::getInstance();
    }
    #endif

    #ifdef TIMER
    TimerMethod* timerMethod = NULL;
    if (isTimerEnabled) {
        timerMethod = TimerMethod::getInstance();
    }
    #endif

    const std::vector<Program>& programs = dataModel.programs();
    std::vector<Program>::const_iterator programIterator = programs.begin();
    for (; programIterator != programs.end(); ++programIterator)
    {


        #ifdef SCOPE
        PicoScopeMeasurement* scopeMeasurement = NULL;
        if (isScopeEnabled) {
            scopeMethod->setSampleRate(100, TIME_US);
            scopeMeasurement = dynamic_cast<PicoScopeMeasurement*>(scopeMethod->start());
        }
        #endif

        #ifdef RAPL
        repara::measurement::Measurement* raplMeasurement = NULL;
        if (isRaplEnabled)
            raplMeasurement = raplMethod->start();
        #endif

        #ifdef TIMER
        repara::measurement::Measurement* timerMeasurement = NULL;
        if (isTimerEnabled) {
            timerMeasurement = timerMethod->start();
        }
        #endif
        // run the current application
        std::cout << "Running the " << (*programIterator).name()  << " program" << std::endl;
        programIterator->runProgram();
        // contains measurement results
        std::string measurementResults;

        #ifdef RAPL
        if (isRaplEnabled && raplMeasurement) {
            // stop the measurement
            raplMeasurement->stop();
            const std::string raplTest = convertKernelSourceMap(*raplMeasurement, "RAPL", isAggregate);
            measurementResults.append(raplTest);
            delete raplMeasurement;
        }
        #endif

        #ifdef TIMER
        if (isTimerEnabled && timerMeasurement) {
            // stop the measurement
            timerMeasurement->stop();
            const std::string timerTest = convertKernelSourceMap(*timerMeasurement, "TIMER", isAggregate);
            measurementResults.append(timerTest);
            delete timerMeasurement;
        }
        #endif

        #ifdef SCOPE
        if (isScopeEnabled && scopeMeasurement) {
            // stop the measurement
            scopeMeasurement->stop();
            const std::string scopeTest = convertKernelSourceMap(*scopeMeasurement, "SCOPE", isAggregate);
            measurementResults.append(scopeTest);
            delete scopeMeasurement;
        }
        #endif
        std::cout << measurementResults << std::endl;

    }

    #ifdef RAPL
    if (raplMethod)
        RaplMethod::deleteInstance();
    #endif

    #ifdef SCOPE
    if (scopeMethod)
        PicoScopeMethod::deleteInstance();
    #endif

    #ifdef TIMER
    if (timerMethod)
        TimerMethod::deleteInstance();
    #endif

    return EXIT_SUCCESS;
}
