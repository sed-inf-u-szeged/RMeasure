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

#ifndef SOURCECAPABILITY_H_INCLUDED
#define SOURCECAPABILITY_H_INCLUDED

namespace repara {
namespace measurement {

/**
 * A type of data a source might be capable of providing after a measurement.
 * Cannot be instantiated directly, the instances available as constant class
 * fields represent the known data types.
 */
class SourceCapability {
    friend class SourceCapabilities;

    typedef int Type;
    Type _value;
    SourceCapability(Type v);

public:
    static const SourceCapability Energy; ///< Capability of energy consumption measurement (in Joules)
    static const SourceCapability MinimumPower; ///< Capability of measuring minimum power dissipation (in Watts)
    static const SourceCapability AveragePower; ///< Capability of measuring average power dissipation (in Watts)
    static const SourceCapability MaximumPower; ///< Capability of measuring maximum power dissipation (in Watts)
    static const SourceCapability ElapsedTime; ///< Capability of elapsed time (a.k.a. wall-clock time) measurement (in seconds)
    static const SourceCapability KernelTime; ///< Capability of measuring CPU-time spent in kernel mode (in seconds)
    static const SourceCapability UserTime; ///< Capability of measuring CPU-time spent in user mode (in seconds)

    /** Check whether two capabilities are equal. */
    bool operator==(SourceCapability that) const;

    /** Check whether two capabilities are not equal. */
    bool operator!=(SourceCapability that) const;

    /** Ordering on capabilities. */
    bool operator<(SourceCapability that) const;

}; // class SourceCapability

/**
 * A set of capabilities.
 */
class SourceCapabilities {
    typedef SourceCapability::Type Type;
    Type _set;
    SourceCapabilities(Type s);

public:
    static const SourceCapabilities None; ///< The empty set of capabilities.

    /** Construct an empty set of capabilities. */
    SourceCapabilities();

    /** Convert a capability to a single-element set. */
    SourceCapabilities(SourceCapability c);

    /** Copy constructor. */
    SourceCapabilities(const SourceCapabilities &s);

    /** Check whether two capability sets are equal. */
    bool operator==(SourceCapabilities that) const;

    /** Check whether two capability sets are not equal. */
    bool operator!=(SourceCapabilities that) const;
    /** Union two sets of capabilities. */
    SourceCapabilities operator|(SourceCapabilities s);

    /** Add (union) a set of capabilities to the currect set. */
    SourceCapabilities& operator|=(SourceCapabilities s);
    /**
     * Check whether the current set of capabilities contains all elements of
     * pattern.
     */
    bool check(SourceCapabilities pattern) const;

}; // class SourceCapabilities

} // namespace repara::measurement
} // namespace repara

#endif // SOURCECAPABILITY_H_INCLUDED
