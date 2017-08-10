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

#include <cstring>
#include <libconfig.h++>
#include <signal.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <fstream>
#include "RMeasureServer.h"

using namespace libconfig;

#ifdef SCOPE
#include <sys/io.h>
#endif

#ifdef RAPL
using namespace rapl;
bool needToCalculate = false;
void alarmHandler(int sig) {
    needToCalculate = true;
    alarm(60);
}
#endif

#ifdef TIMER
using namespace timer;
#endif

void RMeasureServer::callFifo(const char* msg)
{
    int result = access (m_fifoName.c_str(), F_OK);
    if(result == 0) {
        FILE *fp;
        fp = fopen(m_fifoName.c_str(), "w");
        if (fp == NULL) {
            perror("fopen");
            exit(1);
        }
        fputs(msg, fp);

        fclose(fp);
    }
}


void Log (const std::string& logfile,const std::string& message) {
    FILE *file = fopen(logfile.c_str(), "a+");

    if (file){
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,80,"%d/%b/%Y:%H:%M:%S",timeinfo);
        fprintf(file, "[%s]    %s\n", buffer, message.c_str());
        fclose(file);
    }
}

RMeasureServer* RMeasureServer::s_instance = NULL;

RMeasureServer* RMeasureServer::instance()
{
    if (!s_instance)
        s_instance = new RMeasureServer;
    return s_instance;
}

void RMeasureServer::deleteInstance()
{
    delete s_instance;
    s_instance = NULL;
}

RMeasureServer::RMeasureServer() :
    m_measuredKernels(),
    m_isListeningEnabled(false),
    m_scopeListening(false),
    m_raplListening(false),
    m_timerListening(false),
#ifdef RAPL
    m_raplCounter(NULL),
#endif
#ifdef TIMER
    m_timerCounter(NULL),
#endif
#ifdef SCOPE
    m_parallelPortAddress(0xf100),
#endif
    m_portNumber(8081),
    m_logFile("default.log"),
    m_fifoName("RMEASURE_FIFO"),
    m_keepaliveTimeout(0),
    m_keepaliveMaxConn(0),
    m_timeout(15),
    m_dontAdvertise(false),
    m_registry(),
    m_abyssServer(NULL)
{
}

RMeasureServer::~RMeasureServer()
{
#ifdef RAPL
    if (m_raplCounter)
        delete m_raplCounter;
#endif

#ifdef TIMER
    if (m_timerCounter)
        delete m_timerCounter;
#endif

    if (m_abyssServer)
        delete m_abyssServer;
}


void RMeasureServer::listenMacros()
{
    m_isListeningEnabled = true;
    m_measuredKernels.clear();
    umask(0);
    /* Create the FIFO if it does not exist */
    mknod(m_fifoName.c_str(), S_IFIFO|0666, 0);

    #ifdef RAPL
    signal(SIGALRM, alarmHandler);
    alarm(60);
    #endif

    std::string kernelName = "unknown";
    bool isMeasuring = false;
    Log(m_logFile, "Service started to listening via named pipe");

    while (m_isListeningEnabled)
    {
        if (!m_raplListening && !m_scopeListening && !m_timerListening) {
            break;
        }

        #ifdef RAPL
        if (needToCalculate && m_raplListening && isMeasuring) {
            m_raplCounter->calculate();
            needToCalculate = false;
            Log(m_logFile, "calculate() is called to avoid counter overflow!");
        }
        if (!m_raplListening && needToCalculate) {
            needToCalculate = false;
        }
        #endif

        std::ifstream is(m_fifoName.c_str(), std::ifstream::in);     // open file
        std::string msg;

        char c = is.get();
        while (is.good()) {
            if (c == ';') {

                if (msg.compare("E") == 0) {
                    if (isMeasuring) {

                        #ifdef RAPL
                        if (m_raplListening) {
                            m_raplCounter->calculate();
                        }
                        #endif

                        #ifdef SCOPE
                            if (m_scopeListening) {
                                if(ioperm(m_parallelPortAddress,1,1))
                                    Log(m_logFile, "Couldn't open parallel port");
                                else
                                    outb(0x00,m_parallelPortAddress); //set pin1 lo
                            }
                        #endif

                        #ifdef TIMER
                            if (m_timerListening) {
                                m_timerCounter->calculate();
                            }
                        #endif
                        isMeasuring = false;
                    }
                }
                #ifdef SCOPE
                else if (msg.compare("SS") == 0) {
                    scopeListening(false);
                }
                #endif

                #ifdef RAPL
                else if (msg.compare("SR") == 0) {
                    raplListening(false);
                }
                #endif

                #ifdef TIMER
                else if (msg.compare("ST") == 0) {
                    timerListening(false);
                }
                #endif
                else if (!msg.empty()) {
                    std::size_t pos = msg.find("B:");
                    if (pos != std::string::npos) {
                        m_measuredKernels.push_back(msg.substr(pos+2));

                        #ifdef RAPL
                        if (m_raplListening) {
                            m_raplCounter->calculate(true);
                        }
                        #endif

                        #ifdef SCOPE
                        if (m_scopeListening) {
                            if(ioperm(m_parallelPortAddress,1,1))
                                Log(m_logFile, "Couldn't open parallel port");
                            else
                                outb(0x01,m_parallelPortAddress); //set pin1 lo
                        }
                        #endif
                        #ifdef TIMER
                        if (m_timerListening) {
                            m_timerCounter->calculate(true);
                        }
                        #endif
                        isMeasuring = true;
                    }
                }
                else { }
                msg.clear();
            }
            else {
                msg+=c;
            }
            c = is.get();
        }
        is.close();

    }
    m_isListeningEnabled = false;
    Log(m_logFile, "Service stopped to listening via named pipe");
}

#ifdef SCOPE
void RMeasureServer::scopeListening(const bool enabled)
{
    m_scopeListening = enabled;
}
const bool& RMeasureServer::isScopeListening()
{
    return m_scopeListening;
}
#endif


bool RMeasureServer::isListening()
{
    return m_isListeningEnabled;
}

const std::vector<std::string>& RMeasureServer::measuredKernels()
{
    return m_measuredKernels;
}

#ifdef RAPL
void RMeasureServer::raplListening(const bool enabled)
{
    m_raplListening = enabled;
    if (enabled)
        m_raplCounter->startMeasurement();
}

const bool& RMeasureServer::isRaplListening()
{
    return m_raplListening;
}

#endif

#ifdef TIMER
void RMeasureServer::timerListening(const bool enabled)
{
    m_timerListening = enabled;
    if (enabled)
        m_timerCounter->startMeasurement();
}

const bool& RMeasureServer::isTimerListening()
{
    return m_timerListening;
}
#endif

bool RMeasureServer::create(const std::string& configFile)
{
#ifdef RAPL
    std::vector<Processor> v_processors;
#endif

#ifdef TIMER
    std::string systemId;
#endif
    try {
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
            cfg.lookupValue("server.fifoName", m_fifoName);
            cfg.lookupValue("server.keepaliveTimeout", m_keepaliveTimeout);
            cfg.lookupValue("server.keepaliveMaxConn", m_keepaliveMaxConn);
            cfg.lookupValue("server.timeout", m_timeout);
            cfg.lookupValue("server.dontAdvertise", m_dontAdvertise);

#ifdef RAPL
            const Setting& root = cfg.getRoot();
            const Setting &sockets = root["rapl"]["sockets"];
            const int count = sockets.getLength();
            for(int i = 0; i < count; ++i) {
                Processor processor;
                // received socket settings from the config file
                const Setting &socket = sockets[i];
                if(!( socket.lookupValue("hppdl",processor.first)
                       && socket.lookupValue("firstCore", processor.second)))
                    continue;
                v_processors.push_back(processor);
            }
#endif

#ifdef TIMER
            cfg.lookupValue("timer.systemId", systemId);
#endif

#ifdef SCOPE
            cfg.lookupValue("scope.parallelPortAddress", m_parallelPortAddress);
#endif
        }

        xmlrpc_c::methodPtr const StartScopeListeningP(new StartScopeListening);
        xmlrpc_c::methodPtr const StopScopeListeningP(new StopScopeListening);
        m_registry.addMethod("scope.startListening", StartScopeListeningP);
        m_registry.addMethod("scope.stopListening", StopScopeListeningP);

        xmlrpc_c::methodPtr const StartRaplListeningP(new StartRaplListening);
        xmlrpc_c::methodPtr const StopRaplListeningP(new StopRaplListening);
        xmlrpc_c::methodPtr const GetRaplMeasuredDataP(new GetRaplMeasuredData);
        xmlrpc_c::methodPtr const GetMeasuredProcessorsP(new GetMeasuredProcessors);

        m_registry.addMethod("rapl.startListening", StartRaplListeningP);
        m_registry.addMethod("rapl.stopListening", StopRaplListeningP);
        m_registry.addMethod("rapl.getMeasuredData", GetRaplMeasuredDataP);
        m_registry.addMethod("rapl.getMeasuredProcessors", GetMeasuredProcessorsP);

        xmlrpc_c::methodPtr const StartTimerListeningP(new StartTimerListening);
        xmlrpc_c::methodPtr const StopTimerListeningP(new StopTimerListening);
        xmlrpc_c::methodPtr const GetTimerMeasuredDataP(new GetTimerMeasuredData);
        xmlrpc_c::methodPtr const GetMeasuredSystemIdP(new GetMeasuredSystemId);
        m_registry.addMethod("timer.startListening", StartTimerListeningP);
        m_registry.addMethod("timer.stopListening", StopTimerListeningP);
        m_registry.addMethod("timer.getMeasuredData", GetTimerMeasuredDataP);
        m_registry.addMethod("timer.getMeasuredSystemId", GetMeasuredSystemIdP);

        xmlrpc_c::methodPtr const GetMeasuredKernelsP(new GetMeasuredKernels);
        m_registry.addMethod("rmeasure.getMeasuredKernels", GetMeasuredKernelsP);

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

#ifdef RAPL
        if (!m_raplCounter) {
            m_raplCounter = new RaplCounter(v_processors);
        }
        else {
            Log(m_logFile, "RaplCounter is already configured, restart the service to use new configuration for the RaplCounter!");
        }
#endif

#ifdef TIMER
        if (!m_timerCounter) {
            m_timerCounter = new TimerCounter(systemId);
        }
        else {
             Log(m_logFile, "TimerCounter is already configured, restart the service to use new configuration for the TimerCounter!");
        }
#endif
    }
    catch(const FileIOException &fioex)
    {
        Log(m_logFile, "I/O error while reading config file.");
        return false;

    }
    catch(const ParseException &pex)
    {
        std::string message = std::string("Parse error at ") + pex.getFile() + ":" + std::to_string(pex.getLine()) + " - " + pex.getError();
        Log(m_logFile, message);
        return false;
    }
    catch(const SettingNotFoundException &nfex) {
         Log(m_logFile, "Settings not found in config file");
        return false;
    }

    return true;
}

const std::string& RMeasureServer::logFile()
{
    return m_logFile;
}

void RMeasureServer::runOnce()
{
    m_abyssServer->runOnce();
}

#ifdef RAPL
const RaplCounter* RMeasureServer::raplCounter() const
{
    return m_raplCounter;
}
#endif

#ifdef TIMER
const TimerCounter* RMeasureServer::timerCounter() const
{
    return m_timerCounter;
}
#endif

StartScopeListening::StartScopeListening()
{
    this->_signature = "b:";
    this->_help = "This method will start the scope listening";
}

void StartScopeListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef SCOPE
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    // this check ensure only one measurement at once
    if (!rMeasureServer->isScopeListening()) {
        rMeasureServer->scopeListening(true);
        if (!rMeasureServer->isListening())
        {
             std::thread t1 = std::thread(&RMeasureServer::listenMacros, RMeasureServer::instance());
             t1.detach();
        }
        isSucced = true;
        Log(rMeasureServer->logFile(), "Scope started to listening via named pipe");
    }
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}

StartRaplListening::StartRaplListening()
{
    this->_signature = "b:";
    this->_help = "This method will start the rapl listening";
}

void StartRaplListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef RAPL
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    // this check ensure only one measurement at once
    if (!rMeasureServer->isRaplListening()) {
        rMeasureServer->raplListening(true);
        if (!rMeasureServer->isListening())
        {
             std::thread t1 = std::thread(&RMeasureServer::listenMacros, RMeasureServer::instance());
             t1.detach();
        }
        isSucced = true;
        Log(rMeasureServer->logFile(), "Rapl started to listening via named pipe");
    }
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}

StartTimerListening::StartTimerListening()
{
    this->_signature = "b:";
    this->_help = "This method will start the timer listening";
}

void StartTimerListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef TIMER
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    // this check ensure only one measurement at once
    if (!rMeasureServer->isTimerListening()) {
        rMeasureServer->timerListening(true);
        if (!rMeasureServer->isListening())
        {
             std::thread t1 = std::thread(&RMeasureServer::listenMacros, RMeasureServer::instance());
             t1.detach();
        }
        isSucced = true;
        Log(rMeasureServer->logFile(), "Timer started to listening via named pipe");
    }
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}

StopScopeListening::StopScopeListening()
{
    this->_signature = "b:";
    this->_help = "This method will stop the scope listening";
}

void StopScopeListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef SCOPE
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    rMeasureServer->scopeListening(false);

    isSucced = true;
    rMeasureServer->callFifo("SS;");
    Log(rMeasureServer->logFile(), "Scope stopped to listening via named pipe");
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}

StopRaplListening::StopRaplListening()
{
    this->_signature = "b:";
    this->_help = "This method will stop the rapl listening";
}

void StopRaplListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef RAPL
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    rMeasureServer->raplListening(false);
    isSucced = true;
    rMeasureServer->callFifo("SR;");
    Log(rMeasureServer->logFile(), "Rapl stopped to listening via named pipe");
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}

StopTimerListening::StopTimerListening()
{
    this->_signature = "b:";
    this->_help = "This method will stop the timer listening";
}

void StopTimerListening::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    bool isSucced = false;
#ifdef TIMER
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
    rMeasureServer->timerListening(false);
    isSucced = true;
    rMeasureServer->callFifo("ST;");
    Log(rMeasureServer->logFile(), "Timer stopped to listening via named pipe");
#endif
    *retvalP = xmlrpc_c::value_boolean(isSucced);
}


GetRaplMeasuredData::GetRaplMeasuredData()
{
    this->_signature = "A:";
    this->_help = "This method will get the measured data list from the rapl counters";
}

void GetRaplMeasuredData::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    std::vector<xmlrpc_c::value> arrayData;
    RMeasureServer* rMeasureServer = RMeasureServer::instance();

#ifdef RAPL
    const RaplCounter* raplCounter = rMeasureServer->raplCounter();
    if (raplCounter) {
        const KernelList kernelList = raplCounter->kernelList();
        KernelList::const_iterator kernelResultsIt = kernelList.begin();
        for (; kernelResultsIt != kernelList.end(); ++kernelResultsIt) {
            std::map<std::string, xmlrpc_c::value> capsResult;
            MeasurementMap::const_iterator measurementsIt = kernelResultsIt->begin();
            for (; measurementsIt != kernelResultsIt->end(); ++measurementsIt) {
                std::map<std::string, xmlrpc_c::value> measurementValues;
                measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("energy"), xmlrpc_c::value_double(measurementsIt->second.packageEnergy())));
                measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("elapsedTime"), xmlrpc_c::value_double((double)(measurementsIt->second.elapsedTime())/BILLION)));
                capsResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string(measurementsIt->first.first), xmlrpc_c::value_struct(measurementValues)));
            }
            arrayData.push_back(xmlrpc_c::value_struct(capsResult));
        }

        Log(rMeasureServer->logFile(), "Send measured data from the RAPL counters");
    }
    else {
        Log(rMeasureServer->logFile(), "RaplCounter is not defined.");
    }
#else
        Log(rMeasureServer->logFile(), "Send empty measured data from the RAPL counters, because RAPL is undefined");
#endif
     *retvalP = xmlrpc_c::value_array(arrayData);

}


GetTimerMeasuredData::GetTimerMeasuredData()
{
    this->_signature = "A:";
    this->_help = "This method will get the measured data from the timer counter";
}

void GetTimerMeasuredData::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value * const retvalP)
{
    std::vector<xmlrpc_c::value> arrayData;
    RMeasureServer* rMeasureServer = RMeasureServer::instance();

#ifdef TIMER
    const TimerCounter* timerCounter = rMeasureServer->timerCounter();
    if (timerCounter) {

        const ResultList kernelResults = timerCounter->resultList();
        ResultList::const_iterator kernelResultsIt = kernelResults.begin();
        for (; kernelResultsIt != kernelResults.end(); ++kernelResultsIt) {
            std::map<std::string, xmlrpc_c::value> capsResult;
            std::map<std::string, xmlrpc_c::value> measurementValues;
            measurementValues.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string("elapsedTime"), xmlrpc_c::value_double((double)kernelResultsIt->second/BILLION)));
            capsResult.insert(std::pair<std::string, xmlrpc_c::value>(xmlrpc_c::value_string(kernelResultsIt->first), xmlrpc_c::value_struct(measurementValues)));
            arrayData.push_back(xmlrpc_c::value_struct(capsResult));
        }
        Log(rMeasureServer->logFile(), "Send measured data from the Timer counters");
    }
    else {
        Log(rMeasureServer->logFile(), "TimerCounter is not defined.");
    }
#else
        Log(rMeasureServer->logFile(), "Send empty measured data from the TIMER counters, because TIMER is undefined");
#endif
     *retvalP = xmlrpc_c::value_array(arrayData);

}

GetMeasuredProcessors::GetMeasuredProcessors()
{
    this->_signature = "A:";
    this->_help = "This method will send the processors HPP-DL component ref";
}

void GetMeasuredProcessors::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP)
{
    std::vector<xmlrpc_c::value> arrayData;
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
#ifdef RAPL
    const RaplCounter* raplCounter = rMeasureServer->raplCounter();
    if (raplCounter)
    {
        std::vector<Processor> processors = raplCounter->processors();
        std::vector<Processor>::iterator procIt = processors.begin();
        for (; procIt != processors.end(); ++procIt)
            arrayData.push_back(xmlrpc_c::value_string((*procIt).first));

        Log(rMeasureServer->logFile(), "Send processor information");

    }
    else {
        Log(rMeasureServer->logFile(), "Failed to send processor information. RaplCounter is not available");
    }
#else
        Log(rMeasureServer->logFile(), "Send empty measured processors data from the RAPL counters, because RAPL is undefined");
#endif
    *retvalP = xmlrpc_c::value_array(arrayData);

}

GetMeasuredSystemId::GetMeasuredSystemId()
{
    this->_signature = "s:";
    this->_help = "This method will send the measured system id";
}

void GetMeasuredSystemId::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP)
{
    std::string systemId;
    RMeasureServer* rMeasureServer = RMeasureServer::instance();
#ifdef TIMER
    const TimerCounter* timerCounter = rMeasureServer->timerCounter();
    if (timerCounter)
    {
        systemId = timerCounter->systemId();
        Log(rMeasureServer->logFile(), "Send measured system id information");

    }
    else {
        Log(rMeasureServer->logFile(), "Failed to send  measured system id information. TimerCounter is not available");
    }
#else
        Log(rMeasureServer->logFile(), "Send empty measured processors data from the TimerCounter, because TIMER is undefined");
#endif
    *retvalP = xmlrpc_c::value_string(systemId);

}


GetMeasuredKernels::GetMeasuredKernels()
{
    this->_signature = "A:";
    this->_help = "This method will send the measured kernel names";
}

void GetMeasuredKernels::execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP)
{
    std::vector<xmlrpc_c::value> arrayData;
    RMeasureServer* rMeasureServer = RMeasureServer::instance();

    std::vector<std::string> kernels = rMeasureServer->measuredKernels();
    std::vector<std::string>::iterator kernelIt = kernels.begin();
    for (; kernelIt != kernels.end(); ++kernelIt)
        arrayData.push_back(xmlrpc_c::value_string(*kernelIt));

    Log(rMeasureServer->logFile(), "Send a list about the measured kernels name");
    *retvalP = xmlrpc_c::value_array(arrayData);
}
