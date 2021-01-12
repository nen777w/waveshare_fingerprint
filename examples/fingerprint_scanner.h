#ifndef __fingerprint_scanner_h__
#define __fingerprint_scanner_h__

#include <stdint.h>

//fw
#if defined(ADFRUIT_FP)
class Adafruit_Fingerprint;
#else
class waveshare_fingerprint;
#endif

namespace devices
{
    class fingerprint_scanner
    {
    public:
        typedef void(*cb_finger_detected)(void * extra);
        typedef void(*cb_user_detected)(uint16_t id, void * extra);
    public:
#if defined(ADFRUIT_FP)
        fingerprint_scanner(Adafruit_Fingerprint & _device, uint8_t pin_touch, unsigned long out_of_postpone_timeout);
#else
        fingerprint_scanner(waveshare_fingerprint & _device, unsigned long out_of_postpone_timeout);
#endif        

        void register_callback(cb_finger_detected pfn_finger_detected, cb_user_detected pfn_user_detected, void * extra) 
        {
            m_cb_finger_detected = pfn_finger_detected;
            m_cb_user_detected = pfn_user_detected;
            m_extra = extra;
        }

        void update();
        void postpone(bool bpostpone);
        
    private:
#if defined(ADFRUIT_FP)    
        Adafruit_Fingerprint & m_device;
#else
        waveshare_fingerprint & m_device;
#endif
        cb_finger_detected m_cb_finger_detected;
        cb_user_detected m_cb_user_detected;
#if defined(ADFRUIT_FP)        
        uint8_t m_pin_touch;
#endif        
        void * m_extra;
        bool m_finger_detected;
        const unsigned long m_postpone_timeout;
        unsigned long m_postpone;
        bool m_b_postpone;
    };
}

#endif