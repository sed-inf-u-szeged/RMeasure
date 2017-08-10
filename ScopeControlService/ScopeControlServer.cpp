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

#include <libconfig.h++>
#include <vector>

#include "ScopeControlServer.h"

using namespace libconfig;
using namespace ps4000a;

void Log (const std::string& logfile, const std::string& message)
{
    FILE *file = fopen(logfile.c_str(), "a+");

    if (file) {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,80,"%d/%b/%Y:%H:%M:%S",timeinfo);
        //std::string str(buffer);
        fprintf(file, "[%s]    %s\n", buffer, message.c_str());
        fclose(file);
    }
}

ScopeControlServer* ScopeControlServer::s_instance = NULL;

ScopeControlServer* ScopeControlServer::instance()
{
    if (!s_instance)
        s_instance = new ScopeControlServer;
    return s_instance;
}

void ScopeControlServer::deleteInstance()
{
    delete s_instance;
    s_instance = NULL;
}

ScopeControlServer::ScopeControlServer() :
    m_portNumber(8081),
    m_logFile("default.log"),
    m_keepaliveTimeout(0),
    m_keepaliveMaxConn(0),
    m_timeout(15),
    m_dontAdvertise(false),
    m_registry(),
    m_abyssServer(NULL),
    m_picoscope(NULL)
{
}

ScopeControlServer::~ScopeControlServer()
{
    if (m_picoscope)
        delete m_picoscope;
    if (m_abyssServer)
        delete m_abyssServer;
}

bool ScopeControlServer::create(const std::string& configFile)
{
    try {
        ChannelVector channelVector;

        if (!configFile.empty()) {
            Config cfg;
            cfg.readFile(configFile.c_str());

            /*
             * If the setting is found and is of an appropriate type, the value is stored in value
             * and the method returns true.
             * Otherwise, value is left unmodified and the method returns false.
             * These methods do not throw exceptions. We have default values, so in this cases,
             * it's good for us.
            */
            cfg.lookupValue("server.portNumber", m_portNumber);
            cfg.lookupValue("server.logFile", m_logFile);
            cfg.lookupValue("server.keepaliveTimeout", m_keepaliveTimeout);
            cfg.lookupValue("server.keepaliveMaxConn", m_keepaliveMaxConn);
            cfg.lookupValue("server.timeout", m_timeout);
            cfg.lookupValue("server.dontAdvertise", m_dontAdvertise);


            const Setting& root = cfg.getRoot();
            const Setting &channels = root["scope"]["channels"];
            const int count = channels.getLength();
            for(int channelNumber = 0; channelNumber < count; ++channelNumber) {
                // received channel settings from the config file
                const Setting &channel = channels[channelNumber];
                int range = 0, coupling = 0;
                bool enabled = false, parport = false;
                double analogOffset = 0.0, resistance = 0.00, gain = 0.0;
                std::string hppdl;

                /*
                 * If the setting is found and is of an appropriate type, the value is stored in value
                 * and the method returns true.
                 * Otherwise, value is left unmodified and the method returns false.
                 * These methods do not throw exceptions. We have default values, so in this cases,
                 * it's good for us.
                */
                channel.lookupValue("enabled", enabled);
                channel.lookupValue("range", range);
                channel.lookupValue("coupling", coupling);
                channel.lookupValue("analogOffset", analogOffset);
                channel.lookupValue("resistance", resistance);
                channel.lookupValue("gain", gain);
                channel.lookupValue("hppdl", hppdl);
                channel.lookupValue("parport", parport);


                Channel chSettings(channelNumber, hppdl, coupling, range, enabled, analogOffset, resistance, gain, parport);
                channelVector.push_back(chSettings);
            }
        }

        xmlrpc_c::methodPtr const picoOpenMethodP(new PicoOpenMethod);
        xmlrpc_c::methodPtr const picoCloseMethodP(new PicoCloseMethod);
        xmlrpc_c::methodPtr const PicoGetDeviceInfoP(new PicoGetDeviceInfo);
        xmlrpc_c::methodPtr const PicoChannelSettingsP(new PicoChannelSettings);
        xmlrpc_c::methodPtr const PicoStopStreamingP(new PicoStopStreaming);
        xmlrpc_c::methodPtr const PicoStartStreamingP(new PicoStartStreaming);
        xmlrpc_c::methodPtr const PicoGetValuesP(new PicoGetValues);
        xmlrpc_c::methodPtr const PicoRawDataP(new PicoRawData);
        xmlrpc_c::methodPtr const PicoSetSampleP(new PicoSetSample);

        // add XML-RPC methods to the registry
        m_registry.addMethod("pico.open", picoOpenMethodP);
        m_registry.addMethod("pico.close", picoCloseMethodP);
        m_registry.addMethod("pico.getScopeInfo", PicoGetDeviceInfoP);
        m_registry.addMethod("pico.channelInfo", PicoChannelSettingsP);
        m_registry.addMethod("pico.startStreaming", PicoStartStreamingP);
        m_registry.addMethod("pico.stopStreaming", PicoStopStreamingP);
        m_registry.addMethod("pico.getValues", PicoGetValuesP);
        m_registry.addMethod("pico.rawData", PicoRawDataP);
        m_registry.addMethod("pico.setSample", PicoSetSampleP);

        if (!m_abyssServer) {
            /*
             * xmlrpc_c::serverAbyss is an XML-RPC server based on the Abyss HTTP server
             * use the constrOpt paradigm to make specifying options easy and flexible
             * registryP is a pointer to the method registry the server is to use.
             * portNumber gives the TCP port number on which the server is to listen for connections from XML-RPC clients.
             * LogFileName is the file that Abyss will use for a Log file.
             */
            m_abyssServer = new xmlrpc_c::serverAbyss(
                xmlrpc_c::serverAbyss::constrOpt()
                .registryP(&m_registry)
                .portNumber(m_portNumber)
                .logFileName(m_logFile)
                .keepaliveTimeout(m_keepaliveTimeout)
                .keepaliveMaxConn(m_keepaliveMaxConn)
                .timeout(m_timeout)
                .dontAdvertise(m_dontAdvertise)
            );
        }
        else {
            Log(m_logFile, "Server is already configured, restart the service to use new configuration for the Server!");
        }

        if (!m_picoscope)
            m_picoscope = new PicoScope(channelVector);
        else {
            Log(m_logFile, "PicoScope is already configured, restart the service to use new configuration for the Scope!");
        }

    }
    catch(const FileIOException &fioex)
    {
        Log(m_logFile, "I/O error while reading config file.");
        return false;

    }
    catch(const ParseException &pex)
    {
        const std::string message = std::string("Parse error at ") + pex.getFile() + ":" + std::to_string(pex.getLine()) + " - " + pex.getError();
        Log(m_logFile, message);
        return false;
    }
    catch(const SettingNotFoundException &nfex) {
         Log(m_logFile, "Settings not found in config file");
        return false;
    }

    return true;
}

bool ScopeControlServer::openScope()
{
    bool result = false;
    if (m_picoscope) {
        PICO_STATUS status = m_picoscope->openUnit();
        if (status == PICO_OK)
            result = true;
    }
    return result;
}

bool ScopeControlServer::closeScope()
{
    bool result = false;
    if (m_picoscope) {
        PICO_STATUS status = m_picoscope->closeUnit();
        if (status == PICO_OK)
            result = true;
    }
    return result;
}

bool ScopeControlServer::streaming(const bool run)
{
    bool result = false;
    if (m_picoscope) {
        if (run) {
            PICO_STATUS status = m_picoscope->startStreaming();
            if (status == PICO_OK)
                result = true;
        }
        else {
            result = m_picoscope->stopStreaming();
        }
    }
    return result;
}

bool ScopeControlServer::setSampleData(const int sampleInterval, const std::string& sampleUnit)
{
    if (m_picoscope) {
        return m_picoscope->setSampleData(sampleInterval, sampleUnit);
    }
    return false;
}

const std::string& ScopeControlServer::logFile()
{
    return m_logFile;
}

void ScopeControlServer::runOnce()
{
    m_abyssServer->runOnce();
}

const PicoScope* ScopeControlServer::picoScope() const
{
    return m_picoscope;
}


PicoOpenMethod::PicoOpenMethod()
{
    this->_signature = "b:";
    this->_help = "This method opens the scope device";
    }

void PicoOpenMethod::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    bool result = scopeControlServer->openScope();
    if (result)
        Log(scopeControlServer->logFile(), "Scope opened successfully.");
    else
        Log(scopeControlServer->logFile(), "FAILED to open the scope");

    *retvalP = xmlrpc_c::value_boolean(result);
}

PicoCloseMethod::PicoCloseMethod()
{
    this->_signature = "b:";
    this->_help = "This method closes the scope device";
    }

void PicoCloseMethod::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    bool result = scopeControlServer->closeScope();
    if (result)
        Log(scopeControlServer->logFile(), "Scope closed successfully");
    else
        Log(scopeControlServer->logFile(), "FAILED to close the scope");

    *retvalP = xmlrpc_c::value_boolean(result);
}

PicoGetDeviceInfo::PicoGetDeviceInfo()
{
    this->_signature = "S:";
    this->_help = "This method give back a map with specific scope information.";
}

void PicoGetDeviceInfo::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    std::map<std::string, xmlrpc_c::value> infoResult;
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    const PicoScope* picoScope = scopeControlServer->picoScope();
    if (picoScope)
    {
        if (picoScope->scopeUnit()) {
            DeviceInfo deviceInfo = picoScope->scopeUnit()->deviceInfo();
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("driverVersion"), xmlrpc_c::value_string(deviceInfo.driverVersion)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("usbVersion"), xmlrpc_c::value_string(deviceInfo.usbVersion)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("hardwareVersion"), xmlrpc_c::value_string(deviceInfo.hardwareVersion)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("variantNumber"), xmlrpc_c::value_string(deviceInfo.variant)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("batchAndSerial"), xmlrpc_c::value_string(deviceInfo.batchAndSerial)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("calibrationDate"), xmlrpc_c::value_string(deviceInfo.calDate)));
            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("kernelVersion"), xmlrpc_c::value_string(deviceInfo.kernelVersion)));

            Log(scopeControlServer->logFile(), "Send device information");

        }
        else
        {
             Log(scopeControlServer->logFile(), "Failed to send device information. Scope Unit is not available");
        }
    }
    else {
        Log(scopeControlServer->logFile(), "Failed to send device information. PicoScope is not available");
    }
    *retvalP = xmlrpc_c::value_struct(infoResult);

}

PicoChannelSettings::PicoChannelSettings()
{
    this->_signature = "S:";
    this->_help = "This method give back a struct with the information about the scope channels characteristics";
}

void PicoChannelSettings::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    std::map<std::string, xmlrpc_c::value> infoResult;
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    const PicoScope* picoScope = scopeControlServer->picoScope();
    if (picoScope) {
        const ChannelVector chInfo = picoScope->channels();

        ChannelVector::const_iterator channelIterator = chInfo.begin();
        for (; channelIterator != chInfo.end(); ++channelIterator) {
            std::map<std::string, xmlrpc_c::value> channelSettings;
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("hppdl"), xmlrpc_c::value_string(channelIterator->hppdl())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("coupling"), xmlrpc_c::value_int(channelIterator->coupling())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("range"), xmlrpc_c::value_int(channelIterator->rangeInt())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("isEnabled"), xmlrpc_c::value_boolean(channelIterator->isEnabled())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("analogOffset"), xmlrpc_c::value_double(channelIterator->analogOffset())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("resistance"), xmlrpc_c::value_double(channelIterator->resistance())));
            channelSettings.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("isParport"), xmlrpc_c::value_boolean(channelIterator->isParport())));

            infoResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string(channelIterator->channelTypeName()), xmlrpc_c::value_struct(channelSettings)));
        }
        Log(scopeControlServer->logFile(), "Send channel information.");

    }
    else {
         Log(scopeControlServer->logFile(), "Failed to send device information. PicoScope is not available");
    }
    *retvalP = xmlrpc_c::value_struct(infoResult);

}

PicoStartStreaming::PicoStartStreaming()
{
    this->_signature = "b:";
    this->_help = "This method start the streaming mode";
}

void PicoStartStreaming::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    bool returnStatus = scopeControlServer->streaming(true);
    if (returnStatus)
        Log(scopeControlServer->logFile(), "Streaming mode started");
    else
        Log(scopeControlServer->logFile(), "Failed to start streaming mode");

    *retvalP = xmlrpc_c::value_boolean(returnStatus);
}

PicoStopStreaming::PicoStopStreaming()
{
    this->_signature = "b:";
    this->_help = "This method stop the streaming mode";
}

void PicoStopStreaming::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    bool returnStatus = scopeControlServer->streaming(false);
    if (returnStatus)
        Log(scopeControlServer->logFile(), "Streaming mode stopped");
    else
        Log(scopeControlServer->logFile(), "Failed to stop streaming mode");
    *retvalP = xmlrpc_c::value_boolean(returnStatus);
    sleep(1); ///< wait to stop streaming and store data correctly
}

PicoGetValues::PicoGetValues()
{
    this->_signature = "A:";
    this->_help = "This method give back a struct with the results from the measurement.";
}

void PicoGetValues::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    std::vector<xmlrpc_c::value> arrayData;

    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    const PicoScope* picoScope = scopeControlServer->picoScope();
    if (picoScope) {
        if (picoScope->scopeUnit()) {
            const MeasuredValuesList measurementList = picoScope->scopeUnit()->measurementList();
            MeasuredValuesList::const_iterator kernelResultsIt = measurementList.begin();
            for (; kernelResultsIt != measurementList.end(); ++kernelResultsIt) {
                std::map<std::string, xmlrpc_c::value> capsResult;
                MeasurementMap::const_iterator it = kernelResultsIt->first.begin();
                for (; it != kernelResultsIt->first.end(); ++it ) {
                    std::map<std::string, xmlrpc_c::value> measurementValues;
                    measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("energy"), xmlrpc_c::value_double(it->second.energy())));
                    measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("minPower"), xmlrpc_c::value_double(it->second.minPower())));
                    measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("maxPower"), xmlrpc_c::value_double(it->second.maxPower())));
                    measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("elapsedTime"), xmlrpc_c::value_double(it->second.elapsedTime())));

                    capsResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string(it->first.second), xmlrpc_c::value_struct(measurementValues)));
                }
                arrayData.push_back(xmlrpc_c::value_struct(capsResult));
            }
            Log(scopeControlServer->logFile(), "Get results of the measurement.");
        }
        else
            Log(scopeControlServer->logFile(), "Failed to get results of the measurement. ScopeUnit is not available");
    }
    else
        Log(scopeControlServer->logFile(), "Failed to get results of the measurement. PicoScope is not available");

    *retvalP = xmlrpc_c::value_array(arrayData);
}

PicoRawData::PicoRawData()
{
    this->_signature = "A:";
    this->_help = "This method give back a string with the raw data from the measurement.";
}

void PicoRawData::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{

    std::vector<xmlrpc_c::value> arrayData;
    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    const PicoScope* picoScope = scopeControlServer->picoScope();
    if (picoScope) {
        if (picoScope->scopeUnit()) {
            const MeasuredValuesList measurementList = picoScope->scopeUnit()->measurementList();
            MeasuredValuesList::const_iterator kernelResultsIt = measurementList.begin();
            for (; kernelResultsIt != measurementList.end(); ++kernelResultsIt) {
                arrayData.push_back(xmlrpc_c::value_string( kernelResultsIt->second));
            }
            Log(scopeControlServer->logFile(), "Get raw data of the measured kernels.");
        }
        else
            Log(scopeControlServer->logFile(), "Failed to get raw data of the measurement. ScopeUnit is not available");
    }
    else
        Log(scopeControlServer->logFile(), "Failed to get raw data of the measurement. PicoScope is not available");

    *retvalP = xmlrpc_c::value_array(arrayData);
}

PicoSetSample::PicoSetSample()
{
    this->_signature = "b:is";
    this->_help = "This method ensure to configure the sample rating in streaming mode.";
}

void PicoSetSample::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    const int sampleInterval(paramList.getInt(0));
    const std::string sampleUnit(paramList.getString(1));
    paramList.verifyEnd(2);

    ScopeControlServer* scopeControlServer = ScopeControlServer::instance();
    bool returnStatus = scopeControlServer->setSampleData(sampleInterval, sampleUnit);
    if (returnStatus)
        Log(scopeControlServer->logFile(), "Success to set tha sample rating");
    else
        Log(scopeControlServer->logFile(), "Failed to set the sample rating");

    *retvalP = xmlrpc_c::value_boolean(returnStatus);

}
