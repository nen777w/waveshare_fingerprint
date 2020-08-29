#ifndef __waveshare_fingerprint_h__
#define __waveshare_fingerprint_h__

#if defined(WAVESHARE_FP_USE_HW_SERIAL)
	#include <HardwareSerial.h>
	typedef HardwareSerial	Serial_t;
#else
	#include <SoftwareSerial.h>
	typedef SoftwareSerial	Serial_t;
#endif


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
		, 	ACK_NO_TOUCH = 0x09 // Special return code for sleep scanning
	};

	typedef void(*uart_wait_ping_timeout_func_t)(void *);
	typedef void(*verbose_func_t)(const char *);

public:
	waveshare_fingerprint(Serial_t * pss
		, uint8_t pin_reset = -1, uint8_t pin_wake = -1
		, uint16_t uart_wait_timeout = 3000
		, uint16_t reset_timeout = 600
		, uint16_t uart_waut_ping_timeout = 0, uart_wait_ping_timeout_func_t uart_wait_ping_timeout_func = nullptr, void * extra = nullptr
		, verbose_func_t verbose_func = nullptr
	);

	void begin(long speed = 19200);
	void reset();

public:
	bool sleep();
	bool is_sleeping() const { return m_sleep; }
	
	EFPErrors allow_overwrite(bool b);
	
	enum EEnrollStage : uint8_t { stage_0, stage_1, stage_2 };
	EFPErrors enroll_fingerprint(uint16_t slot, uint8_t access_level, EEnrollStage stage);

	EFPErrors remove(uint16_t slot);
	EFPErrors remove_all();

	uint16_t total_fingerprints();

	EFPErrors scan_1_to_1(uint16_t slot);
	EFPErrors scan_1_to_N(uint16_t * slot);
	
	bool begin_sleep_scan();
	EFPErrors sleep_1_to_N_scan(uint16_t * slot, bool stay_in_sleep = true);
	void end_sleep_scan();
	bool is_sleep_scan() const { return m_sleep_scann; }

	EFPErrors permission(uint16_t slot, uint8_t * permission);
	
	EFPErrors get_comparasion_level(uint8_t * level);
	EFPErrors set_comparasion_level(uint8_t level);
	
	EFPErrors get_timeout(uint8_t * timeout);
	EFPErrors set_timeout(uint8_t timeout);
	
	String get_DSP_version();

private:
	void send_command(uint8_t cmd, uint8_t by_2, uint8_t by_3, uint8_t by_4);
	bool read_response(uint8_t * respose, uint16_t response_len);
	void unreset();

private:
	Serial_t * m_pss;
	uint8_t m_pin_reset, m_pin_wake;
	const uint16_t m_uart_wait_timeout;
	const uint16_t m_reset_timeout;
	const uint16_t m_uart_waut_ping_timeout;
	uart_wait_ping_timeout_func_t m_uart_wait_ping_timeout_func;
	void * m_extra;
	bool m_sleep, m_sleep_scann;
	verbose_func_t m_verbose_func;
};

#endif