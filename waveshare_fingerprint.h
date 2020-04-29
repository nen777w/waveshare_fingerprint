#ifndef __waveshare_fingerprint_h__
#define __waveshare_fingerprint_h__

#include <SoftwareSerial.h>

class waveshare_fingerprint
{
public:
	enum EFPErrors : uint8_t
	{
			ACK_SUCCESS = 0x00 // Operation successfully
		,   ACK_FAIL = 0x01 // Operation failed
		,   ACK_FULL = 0x04 // Fingerprint database is full
		,   ACK_NOUSER = 0x05 // No such user
		,   ACK_USER_EXISTS = 0x07 // already exists
		,   ACK_TIMEOUT = 0x08 // Acquisition timeout
	};

public:
	waveshare_fingerprint(SoftwareSerial * pss, uint8_t pin_reset = -1, uint16_t uart_wait_timeout = 3000, bool verbose = false);

	void begin(long speed = 19200);
	void reset();

public:
	bool sleep();
	EFPErrors allow_overwrite(bool b);
	enum EEnrollStage : uint8_t { stage_0, stage_1, stage_2 };
	EFPErrors enroll_fingerprint(uint16_t slot, uint8_t access_level, EEnrollStage stage);
	EFPErrors remove(uint16_t slot);
	EFPErrors remove_all();
	uint16_t total_fingerprints();
	EFPErrors scan_1_to_1(uint16_t slot);
	EFPErrors scan_1_to_N(uint16_t * slot);
	EFPErrors permission(uint16_t slot, uint8_t * permission);
	EFPErrors get_comparasion_level(uint8_t * level);
	EFPErrors set_comparasion_level(uint8_t level);
	EFPErrors get_timeout(uint8_t * timeout);
	EFPErrors set_timeout(uint8_t timeout);
	String get_DSP_version();

private:
	void send_command(uint8_t cmd, uint8_t by_2, uint8_t by_3, uint8_t by_4);
	bool read_response(uint8_t * respose, uint16_t response_len);

private:
	SoftwareSerial * m_pss;
	uint8_t m_pin_reset;
	const uint16_t m_uart_wait_timeout;
	bool m_verbose;
};

#endif