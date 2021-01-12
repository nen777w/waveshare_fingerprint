//ardiono
#include <Arduino.h>
#include "fingerprint_scanner.h"
//gingerprint
#if defined(ADFRUIT_FP)
#include "Adafruit_Fingerprint.h"
#else
#include "waveshare_fingerprint.h"
#endif


namespace devices
{
#if defined(ADFRUIT_FP)
    fingerprint_scanner::fingerprint_scanner(Adafruit_Fingerprint & _device, uint8_t pin_touch, unsigned long out_of_postpone_timeout)
#else
    fingerprint_scanner::fingerprint_scanner(waveshare_fingerprint & _device, unsigned long out_of_postpone_timeout)
#endif
        : m_device(_device)
#if defined(ADFRUIT_FP)        
        , m_pin_touch(pin_touch)
#endif        
        , m_extra(nullptr)
        , m_finger_detected(false)
        , m_postpone_timeout(out_of_postpone_timeout)
        , m_postpone(0)
        , m_b_postpone(false)
    {
#if defined(ADFRUIT_FP)
        pinMode(m_pin_touch, INPUT);
#endif
    }

    void fingerprint_scanner::update()
    {
        if(m_b_postpone) return;
        if(millis() - m_postpone < m_postpone_timeout) return;

#if defined(ADFRUIT_FP)
        bool is_dtected = LOW == digitalRead(m_pin_touch);

        if(!m_finger_detected && is_dtected)
        { //no finger
            m_finger_detected = is_dtected;

            uint8_t code = m_device.getImage();
            if(FINGERPRINT_OK == code)
            {
                code = m_device.image2Tz();
                if(FINGERPRINT_OK == code)
                {
                    code = m_device.fingerFastSearch();
                    if(FINGERPRINT_OK == code)
                    {
                        m_cb_user_detected(m_device.fingerID, m_extra);
                    }
                }
            }

            if(FINGERPRINT_OK != code) 
            {
                m_cb_finger_detected(m_extra);
            }
        }
        else
        {
            m_finger_detected = is_dtected;
        }
#else
        uint16_t slot = 0;
        m_device.sleep_1_to_N_scan(&slot);
        if((uint16_t)-1 == slot) 
        {
            m_cb_finger_detected(m_extra);
        }
        else if(0 < slot)
        {
            m_cb_user_detected(slot, m_extra);
        }
#endif
    }

    void fingerprint_scanner::postpone(bool bpostpone)
    {
        if(false == bpostpone) 
        {
#if !defined(ADFRUIT_FP)
            m_device.begin_sleep_scan();
#endif            
            m_postpone = millis();
        } 
#if !defined(ADFRUIT_FP)
        else 
        {
            m_device.end_sleep_scan();
        }
#endif
        m_b_postpone = bpostpone;
    }
}

