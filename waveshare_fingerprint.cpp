#include "waveshare_fingerprint.h"
#include <Arduino.h>

enum EFPCmd : uint8_t 
{
        CMD_SLEEP = 0x2C
    ,   CMD_SET_MODE = 0x2D
    ,   CMD_ADD_FINGERPRINT_1 = 0x01
    ,   CMD_ADD_FINGERPRINT_2 = 0x02
    ,   CMD_ADD_FINGERPRINT_3 = 0x03
    ,   CMD_DELETE_FINGERPRINT = 0x04
    ,   CMD_DELETE_ALL_FINGERPRINTSS = 0x05
    ,   CMD_GET_FINGERPRINTS_COUNT = 0x09
    ,   CMD_SCAN_COMPARE_1_TO_1 = 0x0B
    ,   CMD_SCAN_COMPARE_1_TO_N = 0x0C
    ,   CMD_READ_PRIVLAGE = 0x0A
    ,   CMD_COMPARISON_LEVEL = 0x28
    ,   CMD_SCAN_GET_IMAGE = 0x24
    ,   CMD_SCAN_GET_EIGENVALS = 0x23
    ,   CMD_SCAN_PUT_EIGENVALS = 0x44
    ,   CMD_PUT_EIGENVALS_COMPARE_1_TO_1 = 0x42
    ,   CMD_PUT_EIGENVALS_COMPARE_1_TO_N = 0x43
    ,   CMD_GET_USER_EIGENVALS = 0x31
    ,   CMD_PUT_USER_EIGENVALS = 0x41
    ,   CMD_GET_USERS_INFO = 0x2B
    ,   CMD_SCAN_TIMEOUT = 0x2E // How long to try scanning - multiples of ~0.25s
    ,   CMD_DSP_VERSION = 0x26
};

struct buffer 
{
    buffer(uint16_t len) {
        data = (uint8_t*)malloc(len);
    }

    ~buffer() {
        free(data);
    }

    uint8_t * data;
};

waveshare_fingerprint::waveshare_fingerprint(SoftwareSerial * pss, uint8_t pin_reset, uint16_t uart_wait_timeout, bool verbose)
    : m_pss(pss)
    , m_pin_reset(pin_reset)
    , m_uart_wait_timeout(uart_wait_timeout)
    , m_verbose(verbose)
{
    
}

void waveshare_fingerprint::begin(long speed)
{
    if(m_pin_reset >= 0) {
        pinMode(m_pin_reset, OUTPUT);
        digitalWrite(m_pin_reset , HIGH);
    }

    m_pss->begin(speed);
}

void waveshare_fingerprint::reset()
{
    if(m_pin_reset >= 0) {
        digitalWrite(m_pin_reset , LOW);
        delay(100);
	    digitalWrite(m_pin_reset , HIGH);
        delay(100);
    }
}

void waveshare_fingerprint::send_command(uint8_t cmd, uint8_t by_2, uint8_t by_3, uint8_t by_4)
{
    uint8_t data[] = { 0xf5, cmd, by_2, by_3, by_4, 0, (uint8_t)((uint8_t)((uint8_t)(cmd ^ by_2) ^ by_3) ^ by_4), 0xf5 };
    if(m_verbose) {
        Serial.print("->");
        for(uint8_t n = 0; n < sizeof(data); n++) { Serial.print(" 0x"); Serial.print(data[n], HEX); }
        Serial.println();
    }
    m_pss->write(data, sizeof(data));
    m_pss->flush();
}

bool waveshare_fingerprint::read_response(uint8_t * respose, uint16_t response_len)
{
    uint16_t timeout = 0;
    uint8_t idx = 0, crc = 0;
    do
    {
        if(m_pss->available()) 
        {
            respose[idx] = m_pss->read();
            if(idx > 0 && idx < response_len-2) crc ^= respose[idx];
            ++idx;
            timeout = 0;
        }
        else {
            delay(1);
            timeout++;
            if(m_uart_wait_timeout < timeout) {
                if(m_verbose) { Serial.println("Error: timeout."); }
                return false;
            }
        }
    } 
    while (idx < response_len);

    if(m_verbose) {
        Serial.print("<-");
        for(uint8_t n = 0; n < response_len; n++) { Serial.print(" 0x"); Serial.print(respose[n], HEX); }
        Serial.println(0xf5 == respose[0] && 0xf5 == respose[response_len-1] && crc == respose[response_len-2] ? " - GOOD PACKET" : " - BAD PACKET");
    }
    
    return 0xf5 == respose[0] && 0xf5 == respose[response_len-1] && crc == respose[response_len-2];
}

bool waveshare_fingerprint::sleep()
{
    uint8_t response[8] = {0};
    send_command(CMD_SLEEP, 0, 0, 0);
    return read_response(response, sizeof(response));
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::allow_overwrite(bool b)
{
    uint8_t response[8] = {0};
    send_command(CMD_SET_MODE, 0, !!!b, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : waveshare_fingerprint::ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::enroll_fingerprint(uint16_t slot, uint8_t access_level, waveshare_fingerprint::EEnrollStage stage)
{
    uint8_t resposne[8] = {0};
    uint8_t u_stage;
    
    switch (stage)
    {
    case stage_0:
            u_stage = CMD_ADD_FINGERPRINT_1;
        break;
    case stage_1:
            u_stage = CMD_ADD_FINGERPRINT_2;
        break;
    case stage_2:
            u_stage = CMD_ADD_FINGERPRINT_3;
        break;
    }

    send_command(u_stage, (slot >> 8) & 0xff, slot & 0xff, access_level & 0x03);
    return read_response(resposne, sizeof(resposne)) ? (waveshare_fingerprint::EFPErrors) resposne[4] : ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::remove(uint16_t slot)
{
    uint8_t response[8] = {0};

    send_command(CMD_DELETE_FINGERPRINT, (slot >> 8) & 0xff, slot & 0xff, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::remove_all()
{
    uint8_t response[8] = {0};
    
    send_command(CMD_DELETE_ALL_FINGERPRINTSS, 0, 0, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : ACK_FAIL;
}

uint16_t waveshare_fingerprint::total_fingerprints()
{
    uint8_t response[8] = {0};

    send_command(CMD_GET_FINGERPRINTS_COUNT, 0, 0, 0);

    if (read_response(response, sizeof(response)) && response[4] == ACK_SUCCESS) {
        return ((((uint16_t) response[2]) << 8) | ((uint16_t) response[3]));
    }
    
    return -1;  
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::scan_1_to_1(uint16_t slot)
{
    uint8_t response[8] = {0};

    send_command(CMD_SCAN_COMPARE_1_TO_1, (slot >> 8) & 0xff, slot & 0xff, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::scan_1_to_N(uint16_t *slot)
{
    uint8_t response[8] = {0};

    send_command(CMD_SCAN_COMPARE_1_TO_N, 0, 0, 0);
    bool read_success = read_response(response, sizeof(response));

    if (read_success)
    {
        switch(response[4])
        {
        case ACK_NOUSER:
                *slot = -1;
            break;
        default:
            {
                *slot = ((((uint16_t) response[2]) << 8) | ((uint16_t) response[3]));
            } break;
        }
    }

    return read_success ? ACK_SUCCESS : ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::permission(uint16_t slot, uint8_t * permission)
{
    uint8_t response[8] = {0};
    send_command(CMD_READ_PRIVLAGE, (slot >> 8) & 0xff, slot & 0xff, 0);    
    if(!read_response(response, sizeof(response))) return ACK_FAIL;

    if(ACK_NOUSER != response[4]) 
    {
        *permission = response[4];
        return ACK_SUCCESS;
    }

    return (waveshare_fingerprint::EFPErrors) response[4];
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::get_comparasion_level(uint8_t * level)
{
    uint8_t response[8] = {0};
    send_command(CMD_COMPARISON_LEVEL, 0, 0, 1);
    if(!read_response(response, sizeof(response))) return ACK_FAIL;

    *level = response[3];
    return (waveshare_fingerprint::EFPErrors) response[4];
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::set_comparasion_level(uint8_t level)
{
    uint8_t response[8] = {0};
    send_command(CMD_COMPARISON_LEVEL, 0, level & 0x09, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : ACK_FAIL;
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::get_timeout(uint8_t * timeout)
{
    uint8_t response[8] = {0};
    send_command(CMD_SCAN_TIMEOUT, 0, 0, 1);
    if(!read_response(response, sizeof(response))) return ACK_FAIL;

    *timeout = response[3];
    return (waveshare_fingerprint::EFPErrors) response[4];
}

waveshare_fingerprint::EFPErrors waveshare_fingerprint::set_timeout(uint8_t timeout)
{
    uint8_t response[8] = {0};
    send_command(CMD_SCAN_TIMEOUT, 0, timeout, 0);
    return read_response(response, sizeof(response)) ? (waveshare_fingerprint::EFPErrors) response[4] : ACK_FAIL;
}

String waveshare_fingerprint::get_DSP_version()
{
    uint8_t response[8] = {0};
    send_command(CMD_DSP_VERSION, 0, 0, 0);
    if(!read_response(response, sizeof(response))) return "";
    
    uint16_t data_len =  (((uint16_t) response[2]) << 8) | ((uint16_t) response[3]);
    buffer buff(data_len + 3);

    if(!read_response(buff.data, data_len+3)) return "";

    String ret;
    for(uint16_t n = 0; n < data_len-1 /*skip first f5*/; ++n) {
        ret += (char)buff.data[1 + n];
    }

    return ret;
}