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

#ifndef PROGRAM_H_INCLUDED
#define PROGRAM_H_INCLUDED

#include "Enviroment.h"

#include <string>

/*
 * Namespace for datamodel implementation(s)
 */
namespace datamodel {

/**
 * Program implementation which contains information about the application
 */
class Program
{
private:
    std::string m_programName; ///< name of the analyzed program
    std::string m_programInputName; ///< it is the name or abbrevation of the given input what the program use during the run

    Enviroment m_enviroment; ///< the used enviroment

    std::string m_targetRunPath; ///<the directory path wherefrom the app will run
    std::string m_targetPath; ///< the program path from the runPath directory
    std::string m_targetInputTarget; ///< the input targets with path from the runPath directory
    std::string m_targetDeviceType; ///< the device type which the application was compiled (eg.: openCL_GPU or openCL_CPU)
public:
    Program();
    void runProgram() const;

    /**
     * \brief A setter function which fill the app name
     * \param name of the analyzed app
     */
    void setProgramName(const std::string& name);

    /**
     * \brief A setter function which fill the name or abbrevation of the given input
     * \param name of the analyzed application
     * \param input what the application use during the run
     */
    void setInputName(const std::string& input);

    /**
     * \brief A setter function which fill the Enviroment fields, it will call the setter function of the Enviroment class
     * \param name of the used enviroment
     * \param version of the used enviroment
     * \param parameters and command options
     */
    void setEnviroment(const std::string& envName, const std::string& envVersion, const std::string& envParams);

    /**
     * \brief A setter function which fill the target information
     * \param runPath is the directory path wherefrom the app will run
     * \param applicationPath is the path of the application from the runPath directory
     * \param inputTarget the input target parameters with the path from the runPath directory
     * \param deviceType is the type of the device which the application was compiled (eg.: openCL_GPU or openCL_CPU)
     */
    void setTargets(const std::string& runPath, const std::string& programPath, const std::string& inputTarget, const std::string& deviceType);

    /**
     * This function will return with the name of the analyzed app
     */
    const std::string& name() const;

    /**
     * This function will return with the device type which the program was compiled
     */
    const std::string& deviceType() const;

    /**
     * This function will return with the name or abbrevation of the given input
     */
    const std::string& inputName() const;

    /**
     * This function will return with the the used enviroment
     */
    const Enviroment& enviroment() const;
};

} // namespace datamodel

#endif // PROGRAM_H_INCLUDED
