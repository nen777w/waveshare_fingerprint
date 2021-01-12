#include "fingerptint_enrollment.h"
#if defined(ADFRUIT_FP)
#include "Adafruit_Fingerprint.h"
#else
#include "waveshare_fingerprint.h"
#endif
//debug / log
//#include "dbg_msg.h"


namespace devices
{
#if defined(ADFRUIT_FP)
    enum {
            MODEL_A = 1
        ,   MODEL_B = 2
    };
#endif    

    void fingerptint_enrollment::start(uint16_t slot, void * extra)
    {
        m_slot = slot;
        m_extra = extra;
#if !defined(ADFRUIT_FP)          
        m_stage = 0;
#endif        

        switch_to(ST_START);
    }

    void fingerptint_enrollment::update()
    {
#if defined(ADFRUIT_FP)        
        switch (m_current_state)
        {
        case ST_START:
            {
                switch_to(ST_WAITING_FOR_FINGER);
            } break;
        case ST_WAITING_FOR_FINGER:
            {
                uint8_t code = m_device.getImage();
                if(FINGERPRINT_OK == code) {
                    code = m_device.image2Tz(MODEL_A);
                    if(FINGERPRINT_OK == code) {
                        switch_to(ST_REMOVE_FINGER);
                    } else {
                        //DBGNL(DBG_ERROR, ("%02x")(code));
                        switch_to(ST_FAIL);
                    }
                }
            } break;
        case ST_REMOVE_FINGER:
            {
                uint8_t code = m_device.getImage();
                if(FINGERPRINT_NOFINGER == code) {
                    switch_to(ST_WAITING_FOR_THE_SAME_FINGER);
                }
            } break;
        case ST_WAITING_FOR_THE_SAME_FINGER:
            {
                uint8_t code = m_device.getImage();
                if(FINGERPRINT_OK == code) {
                    code = m_device.image2Tz(MODEL_B);
                    if(FINGERPRINT_OK != code) {
                        //DBGNL(DBG_ERROR, ("A: %02x")(code));
                        switch_to(ST_FAIL);
                    } else {
                        code = m_device.createModel();
                        if(FINGERPRINT_OK != code) {
                            //DBGNL(DBG_ERROR, ("B: %02x")(code));
                            switch_to(ST_FAIL);
                        } else {
                            code = m_device.storeModel(m_slot);
                            if(FINGERPRINT_OK != code) {
                                //DBGNL(DBG_ERROR, ("C: %02x")(code));
                                switch_to(ST_FAIL);
                            } //if
                            else {
                                switch_to(ST_SUCCESS);
                            }
                        } //else
                    } //else
                } //if
            } break;
        } //switch
#else
    enum {
        def_user_privilage = 1
    };

    switch (m_current_state)
    {
    case ST_START:
        {
            switch_to(ST_WAITING_FOR_FINGER);
        } break;
    case ST_WAITING_FOR_THE_SAME_FINGER:
    case ST_WAITING_FOR_FINGER:
        {
            waveshare_fingerprint::EFPErrors err = m_device.enroll_fingerprint(m_slot, def_user_privilage, (waveshare_fingerprint::EEnrollStage)m_stage);
            switch (err) 
            {
            case waveshare_fingerprint::ACK_SUCCESS:
                    m_stage++;
                    if(m_stage > waveshare_fingerprint::stage_2) {
                        switch_to(ST_SUCCESS);
                    } else {
                        switch_to(ST_REMOVE_FINGER);
                    }
                break;
            case waveshare_fingerprint::ACK_USER_EXISTS:
                    switch_to(ST_USER_EXIST);
                break;
            case waveshare_fingerprint::ACK_FAIL:
                    switch_to(ST_FAIL);
                break;
            default:
                break;
            }
        } break;
    case ST_REMOVE_FINGER:
        {
            switch_to(ST_WAITING_FOR_THE_SAME_FINGER);
        } break;
    }

#endif        
    }
}