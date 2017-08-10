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

#ifndef METHOD_H_INCLUDED
#define METHOD_H_INCLUDED

#include "SourceCapability.h"
#include <map>
#include <string>
#include <vector>

#define SCOPESERVICE "SCOPESERVICE"
#define RMEASURESERVICE "RMEASURESERVICE"

namespace repara {

/**
 * Namespace for the REPARA performance and energy measurement API.
 */
namespace measurement {

/**
 * An abstract representation of a measurement. To be subclassed and returned
 * by measurement method implementations.
 */
class Measurement {
public:
    /**
     * A mapping of SourceCapability (what kind of data was measured) to
     * numeric values (what was measured). Each value in the map is to be
     * interpreted according to its description at SourceCapability.
     */
    typedef std::map<SourceCapability, double> DataMap;

    /**
     * A mapping from HPP-DL component ids (measured where) to
     * DataMap (measured what).
     */
    typedef std::map<std::string, DataMap> SourceMap;


    /*
     * A container of the SourceMaps (measured results of the kernels).
     */
    typedef std::vector<SourceMap> SourceContainer;

    /**
     * A mapping from kernel names to a container of mapping from HPP-DL component ids (measured where) to
     * DataMap (measured what).
     */
    typedef std::map<std::string, SourceContainer> KernelSourceMap;

    /**
     * Virtual destructor, since objects of subclasses will most often be
     * handled and deleted via Measurement pointers.
     */
    virtual ~Measurement() {}

    /**
     * Stop the measurement and process whatever data was collected so that
     * it can be retrieved later by calling kernelSourceMap().
     */
    virtual void stop() = 0;

    /**
     * The results of the measurement stored in a KernelSourceMap. It is not
     * guaranteed to return meaningful data before calling stop().
     */
    virtual const KernelSourceMap& kernelSourceMap() const = 0;

    /**
     * The aggregated results of the given kernel from the KernelSourceMap.
     * It is not guaranteed to return meaningful data before calling stop().
     */
    virtual const SourceMap aggregatedSources(const std::string& kernelName) const = 0;

    /**
     * The container with the results of the given kernel from the KernelSourceMap.
     * It is not guaranteed to return meaningful data before calling stop().
     */
    virtual const SourceContainer kernelSources(const std::string& kernelName) const = 0;

}; // class Measurement

/**
 * An abstract representation of a measurement method. To be subclassed by
 * implementations.
 */
class Method {
public:
    /**
     * A mapping from HPP-DL component ids to sets of capabilities.
     */
    typedef std::map<std::string, SourceCapabilities> SourceCapabilityMap;

    /**
     * Virtual destructor, since objects of subclasses might be handled and
     * deleted via Method pointers.
     */
    virtual ~Method() {}

    /**
     * Describe what hardware components the method can measure and what kind
     * of data it can return for those components.
     */
    virtual const SourceCapabilityMap& sourceCapabilities() const = 0;

    /**
     * Convenience method for retrieving the capabilities of one data source.
     */
    const SourceCapabilities& sourceCapabilities(std::string id) const {
        const SourceCapabilityMap& caps = sourceCapabilities();
        SourceCapabilityMap::const_iterator it = caps.find(id);
        return it != caps.end() ? it->second : SourceCapabilities::None;
    }

    /**
     * Dynamically construct and return an instance of (implementation-subclassed)
     * Measurement at the start of a measurement. The caller has the
     * responsibility to destruct the object when it's not needed anymore.
     * The SourceMap result of calling sources() on the returned Measurement
     * object should be in sync with the SourceCapabilityMap result of calling
     * sourceCapabilities() on this.
     */
    virtual Measurement* start() = 0;

}; // class Method

} // namespace repara::measurement
} // namespace repara

#endif // METHOD_H_INCLUDED
