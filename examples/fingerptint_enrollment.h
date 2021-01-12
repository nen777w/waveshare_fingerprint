#ifndef __fingerptint_enrollment_h__
#define __fingerptint_enrollment_h__

#include <stdint.h>

//fw
#if defined(ADFRUIT_FP)
class Adafruit_Fingerprint;
#else
class waveshare_fingerprint;
#endif

namespace devices
{
    class fingerptint_enrollment
    {
    public:
#if defined(ADFRUIT_FP)    
        explicit fingerptint_enrollment(Adafruit_Fingerprint & _device)
#else
        explicit fingerptint_enrollment(waveshare_fingerprint & _device)
#endif        
            : m_device(_device)
            , m_callback(nullptr)
            , m_current_state(ST_IDLE)
            , m_slot(0)
            , m_extra(nullptr)
        {}

    public:
        enum EEnrollmetStage {
                ST_START
            ,   ST_WAITING_FOR_FINGER
            ,   ST_REMOVE_FINGER
            ,   ST_WAITING_FOR_THE_SAME_FINGER
            ,   ST_SUCCESS
            ,   ST_USER_EXIST
            ,   ST_FAIL
            ,   ST_IDLE
        };

        typedef void(*cb_state_fn)(EEnrollmetStage, void *);
        void register_callback(cb_state_fn callback) { m_callback = callback; }
    
    public:
        void start(uint16_t slot, void * extra);
        void update();
        void reset() { m_current_state = ST_IDLE; }
        bool is_failed() const { return ST_FAIL == m_current_state; }
        bool is_idle() const { return ST_IDLE == m_current_state; }
    private:
        void switch_to(EEnrollmetStage stage) 
        {
            m_current_state = stage;
            m_callback(m_current_state, m_extra);
        }

    private:
#if defined(ADFRUIT_FP)    
        Adafruit_Fingerprint & m_device;
#else
        waveshare_fingerprint & m_device;
        uint8_t m_stage;
#endif        
        cb_state_fn m_callback;
        EEnrollmetStage m_current_state;
        uint16_t m_slot;
        void * m_extra;
    };
}

#endif