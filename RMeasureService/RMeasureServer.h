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

#ifndef RMEASURESERVER_H_INCLUDED
#define RMEASURESERVER_H_INCLUDED

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <vector>

#ifdef RAPL
#include "RaplCounter.h"
#endif

#ifdef TIMER
#include "TimerCounter.h"
#endif

class StartRaplListening : public xmlrpc_c::method {
public:
    StartRaplListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class StopRaplListening: public xmlrpc_c::method {
public:
    StopRaplListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class GetRaplMeasuredData : public xmlrpc_c::method {
public:
    GetRaplMeasuredData();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class StartScopeListening : public xmlrpc_c::method {
public:
    StartScopeListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class StopScopeListening : public xmlrpc_c::method {
public:
    StopScopeListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class GetMeasuredProcessors : public xmlrpc_c::method {
    public:
        GetMeasuredProcessors();
        void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class StartTimerListening : public xmlrpc_c::method {
public:
    StartTimerListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class StopTimerListening: public xmlrpc_c::method {
public:
    StopTimerListening();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class GetTimerMeasuredData : public xmlrpc_c::method {
public:
    GetTimerMeasuredData();
    void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};


class GetMeasuredSystemId : public xmlrpc_c::method {
    public:
        GetMeasuredSystemId();
        void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class GetMeasuredKernels : public xmlrpc_c::method {
    public:
        GetMeasuredKernels();
        void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value* const retvalP);
};

class RMeasureServer {
    std::vector<std::string> m_measuredKernels;
    bool m_isListeningEnabled;
    bool m_scopeListening;
    bool m_raplListening;
    bool m_timerListening;
#ifdef RAPL
    rapl::RaplCounter* m_raplCounter;
#endif
#ifdef TIMER
    timer::TimerCounter* m_timerCounter;
#endif
#ifdef SCOPE
    unsigned int m_parallelPortAddress;
#endif
    unsigned int m_portNumber;
    std::string m_logFile;
    std::string m_fifoName;
    unsigned int m_keepaliveTimeout;
    unsigned int m_keepaliveMaxConn;
    unsigned int m_timeout;
    bool m_dontAdvertise;
    xmlrpc_c::registry m_registry; ///< registry object for the server
    xmlrpc_c::serverAbyss* m_abyssServer;
    static RMeasureServer* s_instance;

    RMeasureServer();
    ~RMeasureServer();
    RMeasureServer(const RMeasureServer&) = delete;
    void operator=(const RMeasureServer&)  = delete;

public:
    static RMeasureServer* instance();
    static void deleteInstance();

    const std::vector<std::string>& measuredKernels();
    bool isListening();
    const std::string& logFile();
    bool create(const std::string& configName = "");
    void runOnce();
    void listenMacros();
#ifdef SCOPE
    void scopeListening(const bool enabled);
    const bool& isScopeListening();
#endif
#ifdef RAPL
    void raplListening(const bool enabled);
    const bool& isRaplListening();
    const rapl::RaplCounter* raplCounter() const;
#endif
#ifdef TIMER
    void timerListening(const bool enabled);
    const bool& isTimerListening();
    const timer::TimerCounter* timerCounter() const;
#endif

void callFifo(const char* msg);

};

#endif //RMEASURESERVER_H_INCLUDED
