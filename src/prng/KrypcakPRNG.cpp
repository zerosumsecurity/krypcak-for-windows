#include "../afx/stdafx.h"
#include "KrypcakPRNG.h"


CKrypcakPRNG::CKrypcakPRNG(void)
{
	//for now, we have not collected any entropy
	m_entropy.m_bOSEntropyBits = 0;
	m_entropy.m_bSysEntropyBits = 0;
	m_entropy.m_bTimingEntropyBits = 0;
	m_entropy.m_bUserEntropyBits = 0;

	//initialize the prng 
	_init_prng();
}


CKrypcakPRNG::~CKrypcakPRNG(void)
{
	//zeroize all internal state buffers
	SecureZeroMemory(m_buffer, SIZE_RANDOM_BUFFER_BYTES);
	SecureZeroMemory(m_state.state, 200);
}

int CKrypcakPRNG::GetRandom( UINT8* output, int num_bytes )
{
	int res=0;

	//we limit the number of bytes to a single output block of the duplex keccak
	//(we simply do not need more in this application)
	VERIFY(num_bytes <= KECCAK_PRNG_IO_SIZE_BYTES);

	//mix in extra random from the windows prng
	_absorb_os_rand();

	//get timing info
	_mix_time_into_entropy_buffer();

	//step the duplexing keccak
	res = Keccak_Duplexing(&m_state, m_buffer, KECCAK_PRNG_IO_SIZE_BYTES, output, num_bytes, KECCAK_SIGMAEND_NO_TRAIL_BIT);

	/*
		make the PRNG forward-secure 
	*/

	//get 128 more bytes of os random to overwrite the internal state of the PRNG
	_absorb_os_rand();	
	
	/*
		done
	*/

	return res;
}

void CKrypcakPRNG::_init_prng()
{
	//zeroize the random buffer
	//(not really needed since we want to accumulate random data
	//in this buffer)
	memset(m_buffer, 0, KECCAK_PRNG_IO_SIZE_BYTES);

	//reset the index into the random input buffer
	m_random_buffer_index = 0;

	//initialize the duplex keccak prng
	Keccak_DuplexInitialize(&m_state, KECCAK_PRNG_RATE_BITS, KECCAK_PRNG_CAPACITY_BITS);

	//mix the tid, pid and disc space info into the pool
	_mix_sys_info_into_entropy_buffer();

	//mix the first random data into the prng
	Update();
}

void CKrypcakPRNG::_absorb_entropy_buffer()
{
	Keccak_Duplexing(&m_state, m_buffer, KECCAK_PRNG_IO_SIZE_BYTES, NULL, 0, KECCAK_SIGMAEND_NO_TRAIL_BIT);
}

void CKrypcakPRNG::_absorb_os_rand()
{
	HCRYPTPROV hProv;
	UINT8 buf[KECCAK_PRNG_IO_SIZE_BYTES];

	//try to open the crypto provider
	if( CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) )
	{
		//if provider is available try to get random
		if( CryptGenRandom( hProv, KECCAK_PRNG_IO_SIZE_BYTES, buf) )
		{
			//mix in the os random 
			Keccak_Duplexing(&m_state, buf, KECCAK_PRNG_IO_SIZE_BYTES, NULL, 0, KECCAK_SIGMAEND_NO_TRAIL_BIT);

			//zeroise the buffer holding the os random
			SecureZeroMemory(buf, KECCAK_PRNG_IO_SIZE_BYTES);

			//see if we have as much os entropy as we estimate we might hope for
			if( (m_entropy.m_bOSEntropyBits + OS_RANDOM_ENTROPY_BITS_PER_CALL) <= MAX_OS_RANDOM_ENTROPY_BITS)
			{
				m_entropy.m_bOSEntropyBits = __min(m_entropy.m_bOSEntropyBits + OS_RANDOM_ENTROPY_BITS_PER_CALL, MAX_OS_RANDOM_ENTROPY_BITS);
			}
		}

		//release the crypto provider
		CryptReleaseContext( hProv, 0 );
	}
}


void CKrypcakPRNG::_mix_time_into_entropy_buffer()
{
	UINT32 time;
	LARGE_INTEGER counter;

	//get a 32-bit tick count (since system boot)
	time = GetTickCount();

	//mix the four bytes into the random input buffer
	MixInRandomByte( UINT8((time>>24)&0xff) );
	MixInRandomByte( UINT8((time>>16)&0xff) );
	MixInRandomByte( UINT8((time>>8 )&0xff) );
	MixInRandomByte( UINT8((time    )&0xff) );

	SYSTEMTIME	st;
	FILETIME	ft;

	//get the current system time
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);	
	time  = (UINT32)ft.dwLowDateTime; 
	time ^= (UINT32)ft.dwHighDateTime;

	//mix the four bytes into the random input buffer
	MixInRandomByte( UINT8((time>>24)&0xff) );
	MixInRandomByte( UINT8((time>>16)&0xff) );
	MixInRandomByte( UINT8((time>>8 )&0xff) );
	MixInRandomByte( UINT8((time    )&0xff) );

	//get the high performance counter
	QueryPerformanceCounter(&counter);

	//mix the four bytes into the random input buffer
	MixInRandomByte( UINT8((counter.LowPart>>24)&0xff) );
	MixInRandomByte( UINT8((counter.LowPart>>16)&0xff) );
	MixInRandomByte( UINT8((counter.LowPart>>8 )&0xff) );
	MixInRandomByte( UINT8((counter.LowPart    )&0xff) );

	//see if we have as much timing info as we can hope for
	if( (m_entropy.m_bTimingEntropyBits + TIMING_INFO_ENTROPY_BITS_PER_CALL) <= MAX_TIMING_INFO_ENTROPY_BITS)
	{
		m_entropy.m_bTimingEntropyBits = __min(m_entropy.m_bTimingEntropyBits + TIMING_INFO_ENTROPY_BITS_PER_CALL, MAX_TIMING_INFO_ENTROPY_BITS);
	}

}

void CKrypcakPRNG::_mix_sys_info_into_entropy_buffer()
{
	UINT32 id;

	id = GetCurrentProcessId();

	//mix the pid into the random input buffer
	MixInRandomByte( (id>>8 )&0xff );
	MixInRandomByte( (id    )&0xff );

	id = GetCurrentThreadId();

	//mix the pid into the random input buffer
	MixInRandomByte( (id>>8 )&0xff );
	MixInRandomByte( (id    )&0xff );

	m_entropy.m_bSysEntropyBits = SYS_PID_ENTROPY_BITS;

	ULARGE_INTEGER a;
	ULARGE_INTEGER b;
	DWORD res;

	if( GetDiskFreeSpaceExW(NULL, NULL, &a, &b) != 0)
	{
		res  = a.LowPart;
		res += a.HighPart;
		res += b.LowPart;
		res += b.HighPart;
		
		//mix the four bytes into the random input buffer
		MixInRandomByte( (res>>24)&0xff );
		MixInRandomByte( (res>>16)&0xff );
		MixInRandomByte( (res>>8 )&0xff );
		MixInRandomByte( (res    )&0xff );

		m_entropy.m_bSysEntropyBits += SYS_DISC_SPACE_ENTROPY_BITS;
	}
}

void CKrypcakPRNG::MixInRandomByte( UINT8 rnd_byte , UINT8 user_entropy)
{
	//inject the random byte at the indexed position
	m_buffer[m_random_buffer_index%SIZE_RANDOM_BUFFER_BYTES] = rnd_byte;

	//step the index
	m_random_buffer_index = (m_random_buffer_index+1);

	//if we have gathered enough entropy from external sources
	if(m_random_buffer_index > (SIZE_RANDOM_BUFFER_BYTES-1))
	{
		//the buffer is 'full', so mix the contents into the keccak state
		_absorb_entropy_buffer();

		//reset the index, so we start mixing in fresh bytes at the beginning
		m_random_buffer_index=0;
	}	

	if( (m_entropy.m_bUserEntropyBits + user_entropy) <= MIN_REQUIRED_ENTROPY_BITS)
	{
		m_entropy.m_bUserEntropyBits += user_entropy;
	}
}

void CKrypcakPRNG::Update()
{
	//get windows random
	_absorb_os_rand();

	//mix in fresh timing data
	_mix_time_into_entropy_buffer();

	//mix the random data into the prng
	_absorb_entropy_buffer();
}

BOOL CKrypcakPRNG::SufficientEntropyGathered()
{
	UINT32 entropy = 0;

	entropy += m_entropy.m_bOSEntropyBits;
	entropy += m_entropy.m_bSysEntropyBits;
	entropy += m_entropy.m_bTimingEntropyBits;
	entropy += m_entropy.m_bUserEntropyBits;
	
	if(entropy < MIN_REQUIRED_ENTROPY_BITS)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
	
}