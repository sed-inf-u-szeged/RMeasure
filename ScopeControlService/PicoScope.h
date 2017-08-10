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

#ifndef PICOSCOPE_H_INCLUDED
#define PICOSCOPE_H_INCLUDED

#include <string>
#include <ps4000aApi.h>
#include <map>
#include <xmlrpc-c/base.hpp>

#include "Channel.h"
#include "MeasurementData.h"

#define BUFFER_SIZE      102400
#define FILTER_NUMBER 3000
#define VOLTAGE 12

/**
 * Namespace for the PicoScope implementation
 */
namespace ps4000a {

/**
* Store information about the scope unit
*/
struct DeviceInfo
{
    std::string driverVersion; ///< version number of PicoScope
    std::string usbVersion; ///< type of USB connection to device
    std::string hardwareVersion; ///< hardware version of device
    std::string variant; ///< variant number of device
    std::string batchAndSerial; ///< bach and serial number of device
    std::string calDate; ///< calibration date of device
    std::string kernelVersion; ///< version of kernel driver
};


class PicoScope {

    class ScopeUnit
    {
        /**
         * A specific list of available scope channels
        */
        enum ChannelNumber
        {
            UNKNOWN = 0,
            DUAL_SCOPE = 2,
            QUAD_SCOPE = 4,
            OCTO_SCOPE = 8
        };

        /**
         * A specific list of available PicosScope models
         */
        enum ModelType
        {
            MODEL_UNKNOWN = 0,
            MODEL_PS4824 = 0x12d8
        };

        int16_t m_handle; ///< the unique identifier for the required device
        bool m_isStreaming; ///< specifies the streaming mode is active or not.
        uint32_t m_sampleInterval; ///< specifies the requested time interval between samples.
        PS4000A_TIME_UNITS m_timeUnit; ///< specifies the unit of time that the sampleInterval is set to.
        int32_t m_sampleCount; ///< the size of the overview buffers.
        bool m_scaleVoltages; ///< specifies the scale voltages is enabled or not.
        MeasuredValuesList m_measurementList; ///< contains the measured values of each kernel in one streaming period
        MeasuredValues m_measuredValues;

        ParPortInfo m_chParPort; ///< specifies channel number and range about the measured parallel port.
        DeviceInfo m_deviceInfo; ///< specifies information about the scope device
        ModelType m_modelType; ///< specifies the current model type
        bool m_external; ///< specifies the model is an external device or not
        PS4000A_RANGE m_firstRange; ///< specifies the minimum possible range value with the current model
        PS4000A_RANGE m_lastRange; ///< specifies the maximum possible range value with the current model
        bool m_signalGenerator; ///< specifies the signalGenerator is enabled
        ChannelNumber m_channelNumber; ///< specifies number of channels on the current scope model
        int16_t* m_buffers[PS4000A_MAX_CHANNEL_BUFFERS * 2]; ///< specifies buffers to receive measurement data

        /** \brief collect streaming values
         *
         * This function is used to collect values while streaming is running.
         */
        void getStreamingValues(const ChannelVector& channels);

        /** \brief ADC to millivolt converter ( if scaleVoltages is enabled)
         * \param raw is the digital data converted from analog signals (ADC value).
         * \param range is specifies the measuring range of the current channel (in milliVolts)
         * \return the millivolt value.
         *
         * Convert an ADC count into millivolts.
         */
        int adc_to_mv(const int16_t& raw, const int& range);

        /*
        * Convert PS4000A_TIME_UNITS into time unit (sec is 1)
        */
        unsigned long long int convertTimeUnit(const PS4000A_TIME_UNITS timeUnit);
        std::string convertTimeUnitToString(const PS4000A_TIME_UNITS timeUnit);

    public:
        ScopeUnit(int16_t handle, const ChannelVector& channels);
        ~ScopeUnit();

        const int16_t& handle() const;
        const DeviceInfo& deviceInfo() const;
        const bool& isStreaming() const;

        /* Get the measured values of the kernels */
        const MeasuredValuesList& measurementList() const;

        /** \brief Run the streaming mode.
         * \param The number of samples in the pre-trigger phase (0 if no trigger has been set).
         *
         * This mode can capture data without the gaps that
         * occur between blocks when using block mode.
         *
         */
        PICO_STATUS runStreaming(const ChannelVector& channels, unsigned long preTrigger = 0);

        /** Set the streaming mode is active or not. */
        void setStreamingMode(const bool& isStreaming);

        void setSampleData(const int& sampleInterval, const PS4000A_TIME_UNITS& sampleUnit);

    };

    ChannelVector m_channels; ///< contains the default channels settings from config file
    ScopeUnit* m_scopeUnit; ///< specifies information about the scope unit.

public:
    PicoScope(const ChannelVector& channels);
    ~PicoScope();

    PICO_STATUS openUnit();
    PICO_STATUS closeUnit();
    PICO_STATUS startStreaming();
    bool stopStreaming();
    bool setSampleData(const int& sampleInterval, const std::string& sampleUnit);

    const ChannelVector& channels() const;
    const ScopeUnit* scopeUnit() const;


};

} // namespace ps4000a

#endif // PICOSCOPE_H_INCLUDED
