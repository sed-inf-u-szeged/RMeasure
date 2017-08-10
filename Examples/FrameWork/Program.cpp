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

#include "Program.h"

namespace datamodel {

Program::Program() :
    m_programName(),
    m_programInputName(),
    m_enviroment(),
    m_targetRunPath(),
    m_targetPath(),
    m_targetInputTarget(),
    m_targetDeviceType()
{
}

void Program::runProgram() const
{
    std::string command = "cd " + m_targetRunPath + ";"
                            + m_targetPath
                            + " "
                            + m_targetInputTarget;
    system(command.c_str());
}

void Program::setProgramName(const std::string& name)
{
    m_programName = name;
}

void Program::setInputName(const std::string& inputName)
{
    m_programInputName = inputName;
}

void Program::setEnviroment(const std::string& envName, const std::string& envVersion, const std::string& envParams)
{
    m_enviroment.setEnviroment(envName, envVersion, envParams);
}

void Program::setTargets(const std::string& runPath, const std::string& programPath, const std::string& inputTarget, const std::string& deviceType)
{
    m_targetRunPath = runPath;
    m_targetPath = programPath;
    m_targetInputTarget = inputTarget;
    m_targetDeviceType = deviceType;
}

const std::string& Program::name() const
{
    return m_programName;
}

const std::string& Program::deviceType() const
{
    return m_targetDeviceType;
}

const std::string& Program::inputName() const
{
    return m_programInputName;
}

const Enviroment& Program::enviroment() const
{
    return m_enviroment;
}

} // namespace datamodel
