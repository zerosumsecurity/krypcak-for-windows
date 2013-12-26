#pragma once

#include "../afx/stdafx.h"
#include <windows.h>
#include <Wincrypt.h>

extern "C"
{
	#include "../keccak/KeccakDuplex.h"
}

#define KECCAK_PRNG_RATE_BITS			1026
#define KECCAK_PRNG_CAPACITY_BITS		574
#define SIZE_RANDOM_BUFFER_BYTES		128
#define KECCAK_PRNG_IO_SIZE_BYTES		128
#define KECCAK_PRNG_IO_SIZE_BITS		(8*KECCAK_PRNG_IO_SIZE_BYTES)


// we want at least 80 bits of entropy, 
#define MIN_REQUIRED_ENTROPY_BITS		80

// we assume a mouse position holds two bits of entropy
// note this means we require 48/2 = 24 mouse positions from the 
// user
#define USER_BIT_ENTROPY_PER_BYTE_MOUSE	  2

// the windows crypto api  
// note will request 128 bytes a lot of times
// the values below indicate we only allow 
#define MAX_OS_RANDOM_ENTROPY_BITS	     64
#define OS_RANDOM_ENTROPY_BITS_PER_CALL	 32

#define MAX_TIMING_INFO_ENTROPY_BITS	 16
#define TIMING_INFO_ENTROPY_BITS_PER_CALL 8

#define SYS_DISC_SPACE_ENTROPY_BITS		 16
#define SYS_PID_ENTROPY_BITS			  8

typedef struct ENTROPY_STATUS{
	UINT8 m_bSysEntropyBits;
	UINT8 m_bOSEntropyBits;
	UINT8 m_bTimingEntropyBits;
	UINT8 m_bUserEntropyBits;
} t_entropy_status;

class CKrypcakPRNG
{
public:
	CKrypcakPRNG(void);
	~CKrypcakPRNG(void);

	/*
		public interface
	*/
	int GetRandom(UINT8* output, int num_bytes);
	void MixInRandomByte(UINT8 rnd_byte, UINT8 user_entropy=0);
	void Update();
	BOOL SufficientEntropyGathered();

private:
	/*
		private methods
	*/
	void _absorb_entropy_buffer();
	void _absorb_os_rand();
	void _mix_time_into_entropy_buffer();
	void _mix_sys_info_into_entropy_buffer();
	void _init_prng();
	
private:
	/*
		private members
	*/
	UINT8 m_buffer[SIZE_RANDOM_BUFFER_BYTES];
	UINT8 m_random_buffer_index;
	Keccak_DuplexInstance m_state;
	
	t_entropy_status m_entropy;

};

