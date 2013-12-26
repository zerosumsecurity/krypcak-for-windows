#include "../afx/stdafx.h"
#include "../core/Core.h"
#include "../threads/KrypcakThreadInfo.h" 

CCore::CCore(void)
{
	//check the keccak implementation to be correct
	if( FALSE == TestCrypto() )
	{
		//crypto not working, bail out
		AfxMessageBox( _T("Integrity check failed.\nUnable to start the application."), MB_ICONERROR );
	}

	//we have no notebook yet
	m_fNotebookLoaded = FALSE;
	//the progress/status window is not here yet 
	m_hWndStatus = NULL;
}


CCore::~CCore(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// interfacing with the PRNG
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	mix a random byte from mouse movement into the PRNG
*/
void CCore::MixInMousePoint( CPoint *pPoint )
{
	BYTE bRand = 0;

	//extract a random byte by concatenating the 4 lsb's
	//of the x-coordinate concatenated with the 4 lsb's
	//of the y-coordinate 
	bRand  = (BYTE)( (pPoint->y)&0xf);
	bRand <<= 4;
	bRand ^= (BYTE)( (pPoint->x)&0xf);

	m_prng.MixInRandomByte(bRand, USER_BIT_ENTROPY_PER_BYTE_MOUSE);
}

/*
	get random data from the PRNG
*/
int CCore::GetRandom( UINT8* output, int num_bytes )
{
	ASSERT(output != NULL);

	//assure we have sufficient entropy in out PRNG
	//if not, get some 
	while( !_sufficent_entropy_gathered() )
	{
		CRandomDlg rand;
		rand.attach(this); 
		rand.DoModal();
	}

	return m_prng.GetRandom(output, num_bytes);	
}

/*
	gently kick the PRNG
*/
void CCore::UpdatePRNG()
{
	m_prng.Update();
}

/*
	ask the PRNG if enough initial entropy has been accumulated
*/
BOOL CCore::_sufficent_entropy_gathered()
{
	return m_prng.SufficientEntropyGathered();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff with the passwords and passphrases
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	generate a password
*/
void CCore::GeneratePassword(CString &strPassword, int password_length, UINT8 bAlphabetMask)
{
	UINT8 buffer[KECCAK_IO_SIZE_BYTES];
	int buf_index = 0;
	strPassword = _T("");
	char c = 0; 
	bool accept_char = false;
	UINT8 alphabet_mask =0;
	bool accept_password =true;

	//get random from the prng
	GetRandom(buffer, KECCAK_IO_SIZE_BYTES);

	//loop while the generated password is not good
	do{
		//empty the password
		strPassword = _T("");
		alphabet_mask = 0;
		
		//if required we will check the password at the end for the all requested character
		//sets to be present - if this check fails, the accept_password check will be set to false
		accept_password = true;

		//loop while our password is not complete yet
		do{
			//if we are at the end of the buffer, fill the buffer with
			//fresh random and reset the pointer 
			if(buf_index > (KECCAK_IO_SIZE_BYTES-1) ) 
			{
				GetRandom(buffer, KECCAK_IO_SIZE_BYTES);
				buf_index = 0;
			}

			//get a random byte from the random buffer
			c = (char)buffer[buf_index++];

			//default to not accept the character
			accept_char = false;
			
			//note we will be throwing a lot candidate chars away
			//the method below is not the most efficient mehtod, but guarantees no biases
			//in theory this loop may not even terminate, but the chance of this happening is zero
			//for every realistic definition of chance. If it does happen, it means our PRNG
			//is producing biased output, and we dont't want to output anyway

			//if it is a digit and we accept digits, this character is ok
			if( (bAlphabetMask&0x01) && isdigit((unsigned char)c) )
			{
				accept_char = true;
				//keep track that we now have a digit in our password
				alphabet_mask |= 0x01;
			}
			//if it is a lower case character and we accept lower case characters, this character is ok
			if( (bAlphabetMask&0x02) && islower((unsigned char)c) ) 
			{
				accept_char = true;
				//keep track that we now have a lower case character in our password
				alphabet_mask |= 0x02;
			}
			//if it is an upper case character and we accept upper case characters, this character is ok
			if( (bAlphabetMask&0x04) && isupper((unsigned char)c) )
			{
				accept_char = true;
				//keep track that we now have an upper case character in our password
				alphabet_mask |= 0x04;
			}	
			//if it is a punctuation character and we accept punctuation characters, this character is ok
			if( (bAlphabetMask&0x08) && ispunct((unsigned char)c) )
			{
				accept_char = true;
				//keep track that we now have a punctuation character in our password
				alphabet_mask |= 0x08;
			}

			//if we have decide to accept he character, add it to the password
			if( accept_char )
			{
				strPassword += ((wchar_t(c)));
			}

		}while(strPassword.GetLength() < password_length);

		//we now have a password of the correct size and with only allowed
		//characters in it. If required, check all requested character sets are present
		if(0x80 == (bAlphabetMask&0x80))
		{
			//check the character set
			if( (bAlphabetMask&0xf) != (alphabet_mask))
			{
				accept_password = false;
			}
		}

	}while(!accept_password);

	//clean the random buffer
	SecureZeroMemory(buffer, KECCAK_IO_SIZE_BYTES);
}

/*
	generate a passphrase
*/
void CCore::GeneratePassphrase( CString &strPassword, UINT32 num_words, UINT8 language_index )
{
	UINT32 i;
	UINT32 index;
	UINT8 rand[KECCAK_IO_SIZE_BYTES];
	strPassword = _T("");

	//get random bytes
	GetRandom(rand, KECCAK_IO_SIZE_BYTES);

	//we have 128 bytes and use two bytes per word in the passphrase
	//hence we can handle passphrases up to length 128/2 = 64
	ASSERT(num_words < MAX_PHRASE_SIZE);
	ASSERT(num_words < (KECCAK_IO_SIZE_BYTES/2) );

	//construct the passphrase word-by-word
	for(i=0; i<num_words-1; i++)
	{
		//truncate 16 bits from the random buffer to 11 bits
		index = (256*rand[2*i] + rand[2*i+1])%2048;

		//add a word to the passphrase, depending on the language
		switch (language_index)
		{
		case DUTCH:
			strPassword += NLWORDS[index];
			break;
		case ENGLISH:
			strPassword += ENWORDS[index];
			break;
		case RFC2289:
			strPassword += RFC2289WORDS[index];
			break;
		default:
			strPassword += ENWORDS[index];
			break;
		}

		//add a space to separate the words
		strPassword += _T(" ");
	}

	//get the index for the last word
	index = (256*rand[2*i] + rand[2*i+1])%2048;

	//add the last word
	switch (language_index)
	{
	case DUTCH:
		strPassword += NLWORDS[index];
		break;
	case ENGLISH:
		strPassword += ENWORDS[index];
		break;
	case RFC2289:
		strPassword += RFC2289WORDS[index];
		break;
	default:
		strPassword += ENWORDS[index];
		break;
	}

	//done, so clean the buffer
	SecureZeroMemory(rand, KECCAK_IO_SIZE_BYTES);
}

/*
	generate a random message id
*/
CString CCore::_get_random_message_id()
{
	CString strID;

	//generate a message id
	GeneratePassword(strID, MSG_ID_SIZE, MSG_ID_ALPHABET_MSK);

	return strID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with initializing Keccak (password hashing)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	initialize the password and salt/iv into a Keccak SpongeWrap object
*/
int CCore::_absorb_password_and_iv(Keccak_DuplexInstance* pSponge, CString* pstrPassword, UINT8 salt[SIZE_IV_IN_BYTES])
{
	ASSERT(pSponge != NULL);
	ASSERT(pstrPassword != NULL);

	int i,len_pswd,num_blocks, last_block, len;
	UINT8* buffer;

	//get the length of the password
	len_pswd = pstrPassword->GetLength();

	//determine the buffer size:
	//buffer -> len(salt) | salt | len(pswd) | pswd
	len = 1 + SIZE_IV_IN_BYTES + 1 + len_pswd*sizeof(wchar_t);

	num_blocks = (len)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = (len)%(UINT64)KECCAK_IO_SIZE_BYTES;

	//note that we can't have last_block = num_blocks = 0
	//since len will be at least 2
	if(0 == last_block)
	{
		num_blocks--;
		last_block = KECCAK_IO_SIZE_BYTES;
	}

	//allocate the buffer
	buffer = new UINT8[len];

	//make sure we start with a clean input buffer
	memset(buffer, 0, len);
	
	//copy the length of the iv/salt
	buffer[0] = SIZE_IV_IN_BYTES;
	//copy the iv /salt
	memcpy(buffer+1, salt, SIZE_IV_IN_BYTES);
	//copy the password length
	buffer[SIZE_IV_IN_BYTES + 1] = (UINT8)len_pswd;
	//copy the password
	memcpy(buffer+ SIZE_IV_IN_BYTES + 2, pstrPassword->GetBuffer(), len_pswd*sizeof(wchar_t));
	
	//absorb the buffer holding all the data
	for(i=0; i<num_blocks; i++)
	{
		if( Keccak_Duplexing(pSponge, buffer + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, NULL, 0, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			return 1;
		}
	}

	//absorb the last block
	if( Keccak_Duplexing(pSponge, buffer + num_blocks*KECCAK_IO_SIZE_BYTES, last_block, NULL, 0, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		return 1;
	}

	//done, so clean the buffer
	SecureZeroMemory(buffer, len);

	delete[] buffer;
	return 0;
}

/*
	set up a Keccak SpongeWrap instance with a password and a salt,
	and get the resulting password check
*/
int CCore::_set_up_crypto(Keccak_DuplexInstance* pSponge, CString* pstrPassword, UINT8 salt[SIZE_IV_IN_BYTES], UINT8 check[PSWD_CHECK_SIZE_BYTES])
{
	ASSERT(pSponge != NULL);
	ASSERT(pstrPassword != NULL);

	_report_progress(KRYPCAK_PROGRESS_MESSAGE, KRYPCAK_PROGRESSMESSAGE_LOAD_PSWD);
	_report_progress(KRYPCAK_PROGRESS_UPDATE, 0);

	int i;
	UINT8* pLargeBuf;
	UINT32 dwStep=0;

	pLargeBuf = new BYTE[SIZE_LARGE_PSWD_HASH_BUFFER];

	//initialize the duplex sponge function
	if( Keccak_DuplexInitialize(pSponge, KECCAK_RATE_BITS, KECCAK_CAPACITY_BITS) )
	{
		return 1;
	}

	//absorb the iv and password into the sponge
	if( _absorb_password_and_iv(pSponge, pstrPassword, salt) )
	{
		return 1;
	}

	//generate a large number of output blocks in duplex/squeeze mode
	for(i=0; i<NUM_ITERATIONS_PSWD_INIT_SQUEEZE; i++)
	{
		if( Keccak_Duplexing(pSponge, NULL, 0, pLargeBuf + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			return 1;
		}

		if(0 == (i%PSWD_PROGRESS_STEPS_TRIGGER))
		{
			dwStep = __min(dwStep+1, NUM_PSWD_PROGRESS_STEPS);
			_report_progress( KRYPCAK_PROGRESS_UPDATE , INCREMENT_PSWD_PROGRESS_STEP*dwStep );
		}
	}

	//mix in the blocks that were generated in the previous block in duplex/absorb mode
	//note the check buffer will hold the 4-byte password check
	for(i=0; i<NUM_ITERATIONS_PSWD_INIT_ABSORB; i++)
	{
		if( Keccak_Duplexing(pSponge, pLargeBuf + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, check, PSWD_CHECK_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			return 1;
		}

		if(0 == (i%PSWD_PROGRESS_STEPS_TRIGGER))
		{
			dwStep = __min(dwStep+1, NUM_PSWD_PROGRESS_STEPS);
			_report_progress( KRYPCAK_PROGRESS_UPDATE , INCREMENT_PSWD_PROGRESS_STEP*dwStep );	
		}
	}

	//clean up the buffer
	SecureZeroMemory(pLargeBuf, SIZE_LARGE_PSWD_HASH_BUFFER);

	delete[] pLargeBuf;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with file encryption and file decryption
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	encrypt a file (high level)
	- creating file objects for reading and writing
	- calling low level routine for actual encryption
*/
int CCore::_encrypt_file( crypt_context *ctx )
{ 
	ASSERT(ctx != NULL);

	CFile f_in;
	CFile f_out;
	CFileException FE;
	BOOL flag;

	TRY
	{
		flag = f_in.Open( *ctx->pstrIn,  CFile::modeRead|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

		if(!flag)
		{
			//something went wrong when reading from the file 
			return ERROR_READ_FROM_FILE;
		}

		TRY
		{
			flag = f_out.Open( *ctx->pstrOut, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive, &FE);
		}
		CATCH(CFileException, e){

			//something went wrong when reading from the file 
			return ERROR_WRITE_TO_FILE;
		}
		END_CATCH

		if(!flag)
		{
			//something went wrong when reading from the file 
			return ERROR_WRITE_TO_FILE;
		}

		int res = _encrypt_file( ctx->pstrPassword, &f_in, &f_out, ctx->iv);

		f_in.Close();
		f_out.Close();

		return res;
}

/*
	encrypt a file (low level routine)
*/
int CCore::_encrypt_file( CString* pstrPassword, CFile* pfIN, CFile* pfOUT, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	ASSERT(pstrPassword != NULL);
	ASSERT(pfIN != NULL);
	ASSERT(pfOUT != NULL);

	Keccak_DuplexInstance spongeWrap;
	UINT8 buf_in[KECCAK_IO_SIZE_BYTES];
	UINT8 buf_out[KECCAK_IO_SIZE_BYTES];
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 header[HEADER_SIZE_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 mac[SIZE_MAC_IN_BYTES];
	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT64 size, num_blocks, trigger;
	UINT32 last_block, dwStep=0, header_index=0;
	UINT32 nread = 0,j=0, i=0;

	//first, get the size of the plain file 
	TRY
	{
		size = pfIN->GetLength();
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//derive the number of full keystream blocks required and the size of the
	//the last incomplete keystream block (could be zero)
	num_blocks = (size)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = ((UINT32)size)%(UINT32)KECCAK_IO_SIZE_BYTES;

	if(0 == last_block)
	{
		if(0 == num_blocks)
		{
			return ERROR_NONE;
		}
		else
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//write the fixed file marker
	TRY
	{
		pfOUT->Write(MARK_ENCFILE, SIZE_MARK_IN_BYTES);
		memcpy(header, MARK_ENCFILE, SIZE_MARK_IN_BYTES);
		header_index = SIZE_MARK_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//write the version
	TRY
	{
		_get_version(DATATYPE_ENCRYPTEDFILE, version);
		pfOUT->Write( version, SIZE_VERSION_IN_BYTES);
		memcpy(header + header_index, version, SIZE_MARK_IN_BYTES);
		header_index += SIZE_VERSION_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH


	//generate a random iv
	GetRandom(iv, SIZE_IV_IN_BYTES);

	//write the iv to file
	TRY
	{
		pfOUT->Write(iv, SIZE_IV_IN_BYTES);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH
	

	memcpy(header + header_index, iv, SIZE_IV_IN_BYTES);
	header_index += SIZE_IV_IN_BYTES;

	//init keccak-duplex with the password and the random iv
	if( _set_up_crypto(&spongeWrap, pstrPassword, iv, check) )
	{
		return ERROR_CRYPTO;
	}

	//write the fixed file marker
	TRY
	{
		pfOUT->Write(check, PSWD_CHECK_SIZE_BYTES);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	memcpy(header + header_index, check, PSWD_CHECK_SIZE_BYTES);
	header_index += PSWD_CHECK_SIZE_BYTES;

	//generate first block of keystream
	if( Keccak_Duplexing(&spongeWrap, header, header_index, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	if(num_blocks > 1000)
	{
		_report_progress(KRYPCAK_PROGRESS_MESSAGE, KRYPCAK_PROGRESSMESSAGE_ENC_FILE);
		_report_progress(KRYPCAK_PROGRESS_UPDATE, 0);
		trigger = num_blocks/10;
	}
	

	for(i=0; i<num_blocks; i++)
	{
		//try to read a full block
		TRY
		{
			nread = pfIN->Read(buf_in, KECCAK_IO_SIZE_BYTES);
		}
		CATCH(CFileException, e){

			//something went wrong when reading from the file 
			SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_READ_FROM_FILE;
		}
		END_CATCH

		//we should have a full block 
		if( KECCAK_IO_SIZE_BYTES != nread )
		{
			SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_READ_FROM_FILE;
		}

		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<nread; j++)
		{
			buf_out[j] = (buf_in[j] ^ ks[j]);
		}

		//write the en/decrypted data to file
		TRY
		{
			pfOUT->Write(buf_out, nread);
		}
		CATCH(CFileException, e){

			//something went wrong when writing to the file 
			SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_WRITE_TO_FILE;
		}
		END_CATCH
			
		//create new ks
		if( Keccak_Duplexing(&spongeWrap, buf_in, nread, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_CRYPTO;
		}

		if( (num_blocks > 1000) && (0 == i%trigger) ) 
		{
			dwStep = __min(dwStep+1,10);
			_report_progress(KRYPCAK_PROGRESS_UPDATE, 10*dwStep);
		}
	}

	//handle the  last block
	TRY
	{
		nread = pfIN->Read(buf_in, last_block);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file
		SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we know how much we should have read
	if(last_block != nread)
	{
		SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_READ_FROM_FILE;
	}

	//xor the key stream bytes with the bytes read from the file
	for(i=0; i<nread; i++)
	{
		buf_out[i] = (buf_in[i] ^ ks[i]);
	}

	SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);

	//write the encrypted data to file
	TRY
	{
		pfOUT->Write(buf_out, nread);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//create he mac
	if( Keccak_Duplexing(&spongeWrap, buf_in, nread, mac, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	SecureZeroMemory(buf_in, KECCAK_IO_SIZE_BYTES);

	//write the mac to file
	TRY
	{
		pfOUT->Write(mac, SIZE_MAC_IN_BYTES);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	
	//all is well
	return ERROR_NONE;
}

/*
	decrypt a file (high level)
	- creating file objects for reading and writing
	- calling low level routine for actual decryption
*/
int CCore::_decrypt_file( crypt_context *ctx )
{
	ASSERT(ctx != NULL);

	CFile f_in;
	CFile f_out;
	CFileException FE;
	BOOL flag;
	CString strTempName;
	int res;

	//try to open the input file
	TRY
	{
		flag = f_in.Open( *ctx->pstrIn,  CFile::modeRead|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//check whether opening went ok
	if(!flag)
	{
		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}

	//get a temporary file name in the same folder as the target file
	strTempName = GetTemporaryFileName(*ctx->pstrOut);

	//try to open the temporary file 
	TRY
	{
		flag = f_out.Open(strTempName, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//check whether opening went ok
	if(!flag)
	{
		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}


	//decrypt the file
	res = _decrypt_file( ctx->pstrPassword, &f_in, &f_out, ctx->iv);

	//close the files
	f_in.Close();
	f_out.Close();

	//if decryption went ok, rename the temp file
	if(ERROR_NONE == res)
	{
		//if the target file already exists, remove it
		TRY 
		{
			CFile::Remove(*ctx->pstrOut);
		}
		CATCH (CFileException, e)
		{
			//the target file did not exist,
			//so just continue
		}
		END_CATCH

			//do the renaming 
			TRY 
		{
			CFile::Rename(strTempName, *ctx->pstrOut);
		}
		CATCH (CFileException, e)
		{
			//something went wrong when writing to the file 
			//try to delete the temp file			
			TRY 
			{
				CFile::Remove(strTempName);
			}
			CATCH (CFileException, e)
			{
				//we are out of options
				//so just return with the error code
			}
			END_CATCH

				return ERROR_WRITE_TO_FILE;
		}
		END_CATCH
	}
	//if here was an error on decryption, clean up the temp file
	else
	{
		TRY 
		{
			CFile::Remove(strTempName);
		}
		CATCH (CFileException, e)
		{
			//we are out of options
			//so just return with the error code
		}
		END_CATCH
	}

	return res;
}

/*
	decrypt a file (low level routine)
*/
int CCore::_decrypt_file( CString* pstrPassword, CFile* pfIN, CFile* pfOUT, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	ASSERT(pstrPassword != NULL);
	ASSERT(pfIN != NULL);
	ASSERT(pfOUT != NULL);

	UINT8 buf_in[KECCAK_IO_SIZE_BYTES];
	UINT8 buf_out[KECCAK_IO_SIZE_BYTES];
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 check_from_file[PSWD_CHECK_SIZE_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 mac[SIZE_MAC_IN_BYTES];
	UINT8 mark[SIZE_MARK_IN_BYTES];
	UINT8 mac_from_file[SIZE_MAC_IN_BYTES];
	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT8 header[HEADER_SIZE_BYTES];
	int header_offset=0;
	UINT32 last_block, dwRes, nread = 0, j=0, i=0;
	UINT64 size, num_blocks, trigger=0;
	Keccak_DuplexInstance sponge;
	UINT32 dwStep=0;

	//first, get the size of the encrypted file 
	TRY
	{
		//get the file size
		size = pfIN->GetLength();
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//the file should be large enough to hold the overhead
	if( size < SIZE_OVERHEAD_IN_BYTES )
	{
		return ERROR_INVALID_FILE;
	}

	//read the mac from the encrypted file
	TRY
	{
		//set file pointer to the beginning of the mac
		pfIN->Seek(-SIZE_MAC_IN_BYTES, CFile::end );
		//read the mac
		nread = pfIN->Read(mac_from_file, SIZE_MAC_IN_BYTES);
		//reset the file pointer
		pfIN->SeekToBegin();
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH



	//we should have read enough bytes to hold a valid mac
	if( nread < SIZE_MAC_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//derive the number of full key stream blocks required and the size of the
	//the last incomplete key stream block (could be zero)
	num_blocks = (size - SIZE_OVERHEAD_IN_BYTES)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = ((UINT32)size - SIZE_OVERHEAD_IN_BYTES)%(UINT32)KECCAK_IO_SIZE_BYTES;

	if(0 == last_block)
	{
		if(0 == num_blocks)
		{
			return ERROR_NONE;
		}
		else
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//try to read the first bytes which should hold the fixed marker
	TRY
	{
		nread = pfIN->Read(mark, SIZE_MARK_IN_BYTES);
		memcpy(header, mark, SIZE_MARK_IN_BYTES);
		header_offset += SIZE_MARK_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we should have read enough bytes to hold a valid marker
	if( nread < SIZE_MARK_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//check the read marker against the fixed marker
	if( memcmp(mark, MARK_ENCFILE, SIZE_MARK_IN_BYTES) )
	{
		return ERROR_INVALID_FILE;
	}

	//try to read the version
	TRY
	{
		nread = pfIN->Read(version, SIZE_VERSION_IN_BYTES);
		memcpy(header + header_offset, version, SIZE_VERSION_IN_BYTES);
		header_offset += SIZE_VERSION_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we should have read enough bytes to hold a valid marker
	if( nread < SIZE_VERSION_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//check the read marker against the fixed marker
	dwRes = _parse_version(DATATYPE_ENCRYPTEDFILE, version);

	if( ERROR_NONE != dwRes )
	{
		return dwRes;
	}

	//try to read the iv from file
	TRY
	{
		nread = pfIN->Read(iv, SIZE_IV_IN_BYTES);
		memcpy(header + header_offset, iv, SIZE_IV_IN_BYTES);
		header_offset += SIZE_IV_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we really should be getting enough bytes for the iv
	if( nread != SIZE_IV_IN_BYTES)
	{
		return ERROR_READ_FROM_FILE;
	}

	//try to read the first bytes which should hold the password check
	TRY
	{
		nread = pfIN->Read(check_from_file, PSWD_CHECK_SIZE_BYTES);
		memcpy(header + header_offset, check_from_file, PSWD_CHECK_SIZE_BYTES);
		header_offset += PSWD_CHECK_SIZE_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH


	//we really should be getting enough bytes for the pswd check
	if( nread != PSWD_CHECK_SIZE_BYTES)
	{
		return ERROR_READ_FROM_FILE;
	}

	//we have the password and the iv, so set up keccak
	if( _set_up_crypto(&sponge, pstrPassword, iv, check) )
	{
		return ERROR_CRYPTO;
	}

	//check the read marker against the fixed marker
	if( memcmp(check, check_from_file, PSWD_CHECK_SIZE_BYTES) )
	{
		return ERROR_INCORRECT_PSWD;
	}

	//generate first block of key stream
	if( Keccak_Duplexing(&sponge, header, header_offset, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	if(num_blocks > 1000)
	{
		_report_progress(KRYPCAK_PROGRESS_MESSAGE, KRYPCAK_PROGRESSMESSAGE_DEC_FILE);
		_report_progress(KRYPCAK_PROGRESS_UPDATE, 0);
		trigger = num_blocks/10;
	}


	//loop over the full key stream blocks
	for(i=0; i<num_blocks; i++)
	{
		//try to read a full output block
		TRY
		{
			nread = pfIN->Read(buf_in, KECCAK_IO_SIZE_BYTES);
		}
		CATCH(CFileException, e){

			//something went wrong when reading from the file 
			SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_READ_FROM_FILE;
		}
		END_CATCH

		//we should have a full block 
		if(KECCAK_IO_SIZE_BYTES != nread)
		{
			SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_READ_FROM_FILE;
		}	

		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<nread; j++)
		{
			buf_out[j] = (buf_in[j] ^ ks[j]);
		}

		//write the encrypted data to file
		TRY
		{
			pfOUT->Write(buf_out, nread);
		}
		CATCH(CFileException, e){
			//something went wrong when writing to the file 
			SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_WRITE_TO_FILE;
		}
		END_CATCH

		//create new key stream
		if( Keccak_Duplexing(&sponge, buf_out, nread, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_CRYPTO;
		}

		if( (num_blocks > 1000) && (0 == i%trigger) ) 
		{
			dwStep = __min(dwStep+1,10);
			_report_progress(KRYPCAK_PROGRESS_UPDATE, 10*dwStep);
		}
	}


	TRY
	{
		nread = pfIN->Read(buf_in, last_block);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we know how much we should have read
	if(last_block != nread)
	{
		SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_READ_FROM_FILE;
	}

	//xor the key stream bytes with the bytes read from the file
	for(i=0; i<nread; i++)
	{
		buf_out[i] = (buf_in[i] ^ ks[i]);
	}

	SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);

	//write the encrypted data to file
	TRY
	{
		pfOUT->Write(buf_out, nread);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//create mac
	if( Keccak_Duplexing(&sponge, buf_out, nread, mac, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	SecureZeroMemory(buf_out, KECCAK_IO_SIZE_BYTES);

	//compare the computed mac with the mac from file
	if(!memcmp(mac, mac_from_file, SIZE_MAC_IN_BYTES))
	{
		return ERROR_NONE;
	}
	else
	{
		return ERROR_CORRUPT_FILE;
	}
}

/*
	find the password for an encrypted file, based on the iv/salt stored in the file
*/
UINT32 CCore::FindPasswordForFile( CString* pstrPassword, CString strURL )
{
	ASSERT(pstrPassword != NULL);

	CFile fIN;
	CFileException FE;
	UINT8 iv[SIZE_IV_IN_BYTES];
	int i,n;

	//if we have no notebook, there is nothing to look for
	if(!m_fNotebookLoaded)
	{
		return ERROR_NO_NOTEBOOK_LOADED;
	}

	//try to open the encrypted file
	TRY
	{
		fIN.Open( strURL,  CFile::modeRead|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

		//try to read the iv from the encrypted file
		if(FALSE == _get_iv_from_encrypted_file(&fIN, iv) )
		{
			return ERROR_READ_FROM_FILE;
			fIN.Close();
		}

		fIN.Close();

		//get the number of file passwords in the loaded notebook
		n = m_notebook.passwords_files.GetSize();

		//try to match the iv
		for(i=0; i<n; i++)
		{
			if( !memcmp(iv,  m_notebook.passwords_files.GetAt(i).iv, SIZE_IV_IN_BYTES) )
			{
				//we have found a match, so get the password
				*pstrPassword = m_notebook.passwords_files.GetAt(i).strPassword;
				return ERROR_NONE;
			}
		}

		return ERROR_PSWD_NOT_FOUND;
}

/*
	get the iv/salt from an encrypted file
*/
BOOL CCore::_get_iv_from_encrypted_file( CFile* fIN, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	ASSERT(fIN != NULL);

	TRY
	{
		//check the file contains an iv
		if( fIN->GetLength() < (SIZE_OVERHEAD_IN_BYTES))
		{
			return FALSE;
		}

		//get to the start of the iv
		fIN->Seek(SIZE_MARK_IN_BYTES + SIZE_VERSION_IN_BYTES, CFile::begin);

		//read the iv
		fIN->Read(iv, SIZE_IV_IN_BYTES);

		//reset the file pointer
		fIN->SeekToBegin();

		return TRUE;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return FALSE;
	}
	END_CATCH
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with message encryption and message decryption
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	encrypt a message and base64-encode the result
*/
UINT32 CCore::_encrypt_and_encode_string( crypt_context* ctx )
{
	ASSERT(ctx != NULL);

	Keccak_DuplexInstance spongeWrap;
	UINT8* buf_input;
	UINT8* buf_output;
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT64 size, num_blocks;
	UINT32 last_block, j=0;
	int i=0, header_offset=0;

	size = sizeof(wchar_t)*(ctx->pstrIn->GetLength());

	//input buffer
	buf_input = (UINT8*)ctx->pstrIn->GetBuffer();

	//output buffer - a bit too large but that is ok
	buf_output = new UINT8[2*(UINT32)size + SIZE_OVERHEAD_IN_BYTES];

	//derive the number of full keystream blocks required and the size of the
	//the last incomplete keystream block (could be zero)
	num_blocks = (size)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = (UINT32)((size)%(UINT64)KECCAK_IO_SIZE_BYTES);

	if(0 == last_block)
	{
		if(0 == num_blocks)
		{
			return ERROR_NONE;
		}
		else
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//generate a random iv
	GetRandom(ctx->iv, SIZE_IV_IN_BYTES);

	//init keccak-duplex with the password and the random iv
	if( _set_up_crypto(&spongeWrap, ctx->pstrPassword, ctx->iv, check) )
	{
		delete[] buf_output;
		return ERROR_CRYPTO;
	}

	memcpy(buf_output+header_offset, MARK_ENCMESSAGE, SIZE_MARK_IN_BYTES);
	header_offset += SIZE_MARK_IN_BYTES;

	_get_version(DATATYPE_ENCRYPTEDMESSAGE, version);
	memcpy(buf_output+header_offset, version, SIZE_VERSION_IN_BYTES);
	header_offset += SIZE_VERSION_IN_BYTES;

	//write the iv to buffer
	memcpy(buf_output+header_offset, ctx->iv, SIZE_IV_IN_BYTES);
	header_offset += SIZE_IV_IN_BYTES;

	//write the password check
	memcpy(buf_output+header_offset, check, PSWD_CHECK_SIZE_BYTES);
	header_offset += PSWD_CHECK_SIZE_BYTES;

	//absorb the header and generate first block of keystream
	if( Keccak_Duplexing(&spongeWrap, buf_output, header_offset, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		delete[] buf_output;
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);

		return ERROR_CRYPTO;
	}

	//handle the complete blocks with a '1' trailing bit
	for(i=0; i<num_blocks; i++)
	{
		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<KECCAK_IO_SIZE_BYTES; j++)
		{
			buf_output[j + header_offset + i*KECCAK_IO_SIZE_BYTES] = (buf_input[j +  i*KECCAK_IO_SIZE_BYTES] ^ ks[j]);
		}

		//create new ks
		if( Keccak_Duplexing(&spongeWrap, buf_input +  i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			delete[] buf_output;
			return ERROR_CRYPTO;
		}
	}

	//the last block with a trailing '0' bit
	for(j=0; j<last_block; j++)
	{
		buf_output[j + header_offset + num_blocks*KECCAK_IO_SIZE_BYTES] = (buf_input[j +  num_blocks*KECCAK_IO_SIZE_BYTES] ^ ks[j]);
	}

	SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);

	//create mac
	if( Keccak_Duplexing(&spongeWrap, buf_input+num_blocks*KECCAK_IO_SIZE_BYTES, last_block, buf_output+header_offset+size, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		delete[] buf_output;
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	SecureZeroMemory(buf_input, (UINT32)size);

	_base64_encode(buf_output, header_offset + (UINT32)size + SIZE_MAC_IN_BYTES, *ctx->pstrOut);

	delete[] buf_output;

	//all is well
	return ERROR_NONE;
}

/*
	base64-decode an encrypted message and decrypt the result
*/
UINT32 CCore::_decode_and_decrypt( crypt_context *ctx )
{
	UINT32 dwLength;
	UINT8* pBuf=NULL;
	UINT8* pBufOut=NULL;
	wchar_t* tmp;
	ASSERT(ctx != NULL);

	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 check_from_file[PSWD_CHECK_SIZE_BYTES];
	UINT8 mac[SIZE_MAC_IN_BYTES];
	UINT8 mark[SIZE_MARK_IN_BYTES];
	UINT8 mac_from_message[SIZE_MAC_IN_BYTES];
	int i=0;
	UINT32 j=0, index=0, offset =0, last_block;
	UINT64 size, num_blocks;
	Keccak_DuplexInstance spongeWrap;
	UINT32 res;

	dwLength = (sizeof(wchar_t))*ctx->pstrIn->GetLength();

	pBuf = new UINT8[dwLength];

	res = _base64_decode(ctx->pstrIn, pBuf, &dwLength);


	if(ERROR_NONE != res)
	{
		delete [] pBuf;
		return res;
	}

	if(dwLength < SIZE_OVERHEAD_IN_BYTES)
	{
		delete[] pBuf;
		return ERROR_CORRUPT_MESSAGE;
	}

	pBufOut = new UINT8[dwLength];

	size = dwLength;
	index = 0;
	offset = 0;

	//get the mac as included in the encrypted message buffer
	memcpy(mac_from_message, pBuf + (dwLength - SIZE_MAC_IN_BYTES), SIZE_MAC_IN_BYTES);

	//get the number of blocks and the size of the remaining block
	num_blocks = (size - SIZE_OVERHEAD_IN_BYTES)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = (UINT32)((size - SIZE_OVERHEAD_IN_BYTES)%(UINT64)KECCAK_IO_SIZE_BYTES);

	//make sure the last block is non-empty
	if(0 == last_block)
	{
		//adjust the last data block so it is non-empty
		//(if required and possible)
		if(num_blocks > 0)
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//get the marker
	memcpy(mark, pBuf + offset, SIZE_MARK_IN_BYTES);
	offset += SIZE_MARK_IN_BYTES;

	//check the read marker against the fixed marker
	if( memcmp(mark, MARK_ENCMESSAGE, SIZE_MARK_IN_BYTES) )
	{
		delete[] pBuf;
		delete[] pBufOut;
		return ERROR_CORRUPT_MESSAGE;
	}

	//get the version
	memcpy(version, pBuf + offset, SIZE_VERSION_IN_BYTES);
	offset += SIZE_VERSION_IN_BYTES;

	//parse the version and see if we can handle it
	res = _parse_version(DATATYPE_ENCRYPTEDMESSAGE, version);
	if( ERROR_NONE != res )
	{
		delete[] pBuf;
		delete[] pBufOut;

		return res;
	}

	//read the iv from file
	memcpy(ctx->iv, pBuf + offset, SIZE_IV_IN_BYTES);
	offset += SIZE_IV_IN_BYTES;

	//read the password check 
	memcpy(check_from_file, pBuf + offset, PSWD_CHECK_SIZE_BYTES);
	offset += PSWD_CHECK_SIZE_BYTES;

	//we have the password and the iv, so set up keccak
	if( _set_up_crypto(&spongeWrap, ctx->pstrPassword, ctx->iv, check) )
	{
		delete[] pBuf;
		delete[] pBufOut;
		return ERROR_CRYPTO;
	}

	//verify the password check
	if( memcmp(check, check_from_file, PSWD_CHECK_SIZE_BYTES) )
	{
		delete[] pBuf;
		delete[] pBufOut;
		return ERROR_INCORRECT_PSWD;
	}

	//generate first block of key stream
	if( Keccak_Duplexing(&spongeWrap, pBuf, offset, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		delete[] pBuf;
		delete[] pBufOut;
		return ERROR_CRYPTO;
	}

	//loop over the full key stream blocks
	for(i=0; i<num_blocks; i++)
	{
		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<KECCAK_IO_SIZE_BYTES; j++)
		{
			pBufOut[j + i*KECCAK_IO_SIZE_BYTES] = (pBuf[j + offset + i*KECCAK_IO_SIZE_BYTES] ^ ks[j]);
		}

		//create new ks
		if( Keccak_Duplexing(&spongeWrap, pBufOut + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			delete[] pBuf;
			delete[] pBufOut;
			return ERROR_CRYPTO;
		}
	}

	//the last block with a trailing '0' bit
	for(j=0; j<last_block; j++)
	{
		pBufOut[j + num_blocks*KECCAK_IO_SIZE_BYTES] = (pBuf[j + offset + num_blocks*KECCAK_IO_SIZE_BYTES] ^ ks[j]);
	}

	//create mac
	if( Keccak_Duplexing(&spongeWrap, pBufOut+num_blocks*KECCAK_IO_SIZE_BYTES, last_block, mac, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		delete[] pBuf;
		delete[] pBufOut;
		return ERROR_CRYPTO;
	}


	//compare the computed mac with the mac from file
	if(!memcmp(mac, mac_from_message, SIZE_MAC_IN_BYTES))
	{
		dwLength = ((UINT32)num_blocks*KECCAK_IO_SIZE_BYTES + last_block)/(sizeof(wchar_t));
		tmp = (wchar_t*)pBufOut;
		*ctx->pstrOut = (CString(tmp).Left(dwLength));

		delete[] pBufOut;
		delete[] pBuf;

		return ERROR_NONE;
	}
	else
	{
		delete[] pBufOut;
		delete[] pBuf;

		return ERROR_CORRUPT_MESSAGE;
	}

	return ERROR_NONE;
}
/*
	pre-parse encrypted message 
*/
UINT32 CCore::PreParseEncryptedMessage( CString *pstrMessage, CString &strID, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	int len, index;
	UINT32 dwLen, dwRes;
	CString strMsg;
	UINT8 buf[SIZE_MARK_IN_BYTES+SIZE_VERSION_IN_BYTES+SIZE_IV_IN_BYTES];

	//try to find the message id in the encrypted base64 encoded message
	//if none found, return "<unknown>"
	if(ERROR_NONE != _find_message_id( strID, pstrMessage ))
	{
		strID = _T("<unknown>");
	}

	//remove redundant padding from the message
	pstrMessage->Replace(MSG_HEADER, _T(""));
	pstrMessage->Replace(MSG_FOOTER, _T(""));
	pstrMessage->Remove(_T('\r'));
	pstrMessage->Remove(_T('\n'));
	pstrMessage->Remove(_T(' '));

	len = pstrMessage->GetLength();
	index = pstrMessage->Find( _T("KRYP>") );

	//make sure our message holds the "KRYP>" marker and is large enough to hold an iv
	if( (index < 0) || (len-index) < 2*SIZE_IV_IN_BYTES )
	{
		return ERROR_CORRUPT_MESSAGE;
	}

	strMsg = pstrMessage->Mid(index, 2*SIZE_IV_IN_BYTES);

	//decode the part that holds the marker, version and iv
	dwRes = _base64_decode( &strMsg, buf, &dwLen);

	//copy the iv
	if(ERROR_NONE == dwRes)
	{
		memcpy(iv, buf+SIZE_MARK_IN_BYTES+SIZE_VERSION_IN_BYTES, SIZE_IV_IN_BYTES);
	}

	//done
	return dwRes;
}


/*
	find the message id in a base64-encoded and encrypted message
*/
UINT32 CCore::_find_message_id( CString &strID, CString* pstrBase64EncodedMessage )
{
	//find the message id header
	int index = pstrBase64EncodedMessage->Find(MSG_ID_HEADER);

	//if we have found the message id header and there is room in the base64-encode encrypted message 
	//for the message id
	if( (index>0) && (index < pstrBase64EncodedMessage->GetLength()- MSG_ID_SIZE) )
	{
		//read the message id
		strID = pstrBase64EncodedMessage->Mid(index + MSG_ID_HEADER.GetLength(),  MSG_ID_SIZE);
		strID.Replace(_T(" "), _T(""));
		strID.Replace(_T("\r"), _T(""));
		strID.Replace(_T("\n"), _T(""));

		return ERROR_NONE;
	}

	//no message id found
	return ERROR_NO_MSG_ID;
}

/*
	base-64 encode a string
*/
UINT32 CCore::_base64_encode( UINT8 *buf, UINT32 dwInputLength, CString &strOutput )
{
	int i = 0;
	int j = 0;
	UINT8 char_array_3[3];
	UINT8 char_array_4[4];
	strOutput = _T("");

	i=0;

	while (dwInputLength--) {
		char_array_3[i++] = *(buf++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				strOutput += BASE64_ALPHABET[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
		{
			char_array_3[j] = _T('\0');
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			strOutput += BASE64_ALPHABET[char_array_4[j]];

		while((i++ < 3))
			strOutput += _T('=');
	}

	return ERROR_NONE;
}

/*
	base-64 decode a string
*/
UINT32 CCore::_base64_decode( CString *pstrInput, UINT8* buf, UINT32 *dwOutputLength )
{
	size_t in_len = pstrInput->GetLength();
	size_t i = 0;
	size_t j = 0;
	int in_ = 0;
	UINT32 index=0;
	wchar_t input[4];
	UINT8 indices_input[4];
	int tmp;


	while (in_len-- && ( pstrInput->GetAt(in_) != _T('=') )  ) 
	{
		input[i++] = pstrInput->GetAt(in_); 
		in_++;
		if (i ==4) 
		{
			for (i = 0; i <4; i++)
			{
				tmp = BASE64_ALPHABET.Find(input[i]);

				if(-1 == tmp)
				{
					return ERROR_DECODE_BASE64;
				}
				else
				{
					indices_input[i] = (UINT8)(tmp);
				}
			}

			buf[index++] = (indices_input[0] << 2) + ((indices_input[1] & 0x30) >> 4);
			buf[index++] = ((indices_input[1] & 0xf) << 4) + ((indices_input[2] & 0x3c) >> 2);
			buf[index++] = ((indices_input[2] & 0x3) << 6) + indices_input[3];

			i=0;
		}
	}

	if(i)
	{
		*dwOutputLength = i+index-1;
	}
	else
	{
		*dwOutputLength = index;
	}

	if (i) 
	{
		for (j = i; j <4; j++)
		{
			input[j] = 0;
		}

		for (j = 0; j <4; j++)
		{
			tmp = (UINT8)BASE64_ALPHABET.Find(input[j]);

			if(-1 == tmp)
			{
				return ERROR_DECODE_BASE64;
			}
			else
			{
				indices_input[j] = (UINT8)(tmp);
			}
		}

		buf[index++] = (indices_input[0] << 2) + ((indices_input[1] & 0x30) >> 4);
		buf[index++] = ((indices_input[1] & 0xf) << 4) + ((indices_input[2] & 0x3c) >> 2);
		buf[index++] = ((indices_input[2] & 0x3) << 6) + indices_input[3];

	}

	return ERROR_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with notebook encryption and decryption
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCore::_encrypt_buffer_to_file(CString* pstrPassword, UINT8* buf, UINT32 num_bytes, CFile* pfOUT )
{
	ASSERT(pstrPassword != NULL);
	ASSERT(pfOUT != NULL);
	ASSERT(buf != NULL);

	Keccak_DuplexInstance spongeWrap;
	UINT8 buf_out[KECCAK_IO_SIZE_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 iv[SIZE_IV_IN_BYTES];
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 mac[SIZE_MAC_IN_BYTES];
	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT8 header[HEADER_SIZE_BYTES];
	UINT64 num_blocks;
	UINT32 last_block,header_offset=0;
	UINT32 i=0, j=0;

	
	//derive the number of full keystream blocks required and the size of the
	//the last incomplete keystream block (could be zero)
	num_blocks = (num_bytes)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = (num_bytes)%(UINT32)KECCAK_IO_SIZE_BYTES;

	if(0 == last_block)
	{
		if(0 == num_blocks)
		{
			return ERROR_NONE;
		}
		else
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//write the fixed file marker
	TRY
	{
		pfOUT->Write(MARK_ENCNOTEBOOK, SIZE_MARK_IN_BYTES);
		memcpy(header, MARK_ENCNOTEBOOK, SIZE_MARK_IN_BYTES);
		header_offset += SIZE_MARK_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//write the version
	TRY
	{
		_get_version(DATATYPE_NOTEBOOK, version);
		pfOUT->Write( version, SIZE_VERSION_IN_BYTES);
		memcpy(header + header_offset, version, SIZE_VERSION_IN_BYTES);
		header_offset += SIZE_VERSION_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//generate a random iv
	GetRandom(iv, SIZE_IV_IN_BYTES);

	//write the iv to file
	TRY
	{
		pfOUT->Write(iv, SIZE_IV_IN_BYTES);
		memcpy(header + header_offset, iv, SIZE_IV_IN_BYTES);
		header_offset += SIZE_IV_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//init keccak-duplex with the password and the random iv
	if( _set_up_crypto(&spongeWrap, pstrPassword, iv, check) )
	{
		return ERROR_CRYPTO;
	}

	//write the fixed file marker
	TRY
	{
		pfOUT->Write(check, PSWD_CHECK_SIZE_BYTES);
		memcpy(header + header_offset, check, PSWD_CHECK_SIZE_BYTES);
		header_offset += PSWD_CHECK_SIZE_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//generate first block of keystream
	if( Keccak_Duplexing(&spongeWrap, header, header_offset, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
		return ERROR_CRYPTO;
	}

	for(i=0; i<num_blocks; i++)
	{

		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<KECCAK_IO_SIZE_BYTES; j++)
		{
			buf_out[j] = (buf[j + i*KECCAK_IO_SIZE_BYTES] ^ ks[j]);
		}

		//write the en/decrypted data to file
		TRY
		{
			pfOUT->Write(buf_out, KECCAK_IO_SIZE_BYTES);
		}
		CATCH(CFileException, e){

			//something went wrong when writing to the file 
			SecureZeroMemory(ks, KECCAK_IO_SIZE_BYTES);
			return ERROR_WRITE_TO_FILE;
		}
		END_CATCH

		//create new ks
		if( Keccak_Duplexing(&spongeWrap, buf + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			return ERROR_CRYPTO;
		}
	}

	//xor the key stream bytes with the bytes read from the file
	for(i=0; i<last_block; i++)
	{
		buf_out[i] = (buf[i + num_blocks*KECCAK_IO_SIZE_BYTES] ^ ks[i]);
	}

	//write the encrypted data to file
	TRY
	{
		pfOUT->Write(buf_out, last_block);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//create mac
	if( Keccak_Duplexing(&spongeWrap, buf + num_blocks*KECCAK_IO_SIZE_BYTES, last_block, mac, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		return ERROR_CRYPTO;
	}
	
	//write the mac to file
	TRY
	{
		pfOUT->Write(mac, SIZE_MAC_IN_BYTES);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH

	//all is well
	return ERROR_NONE;
}


int CCore::_decrypt_file_into_buffer( CString* pstrPassword, UINT8* buf, UINT32 &num_bytes, CFile* pfIN )
{
	ASSERT(pstrPassword != NULL);
	ASSERT(buf != NULL);
	ASSERT(pfIN != NULL);

	UINT8 buf_in[KECCAK_IO_SIZE_BYTES];
	UINT8 ks[KECCAK_IO_SIZE_BYTES];
	UINT8 iv[SIZE_IV_IN_BYTES];
	UINT8 check[PSWD_CHECK_SIZE_BYTES];
	UINT8 check_from_file[PSWD_CHECK_SIZE_BYTES];
	UINT8 mac[SIZE_MAC_IN_BYTES];
	UINT8 mark[SIZE_MARK_IN_BYTES];
	UINT8 mac_from_file[SIZE_MAC_IN_BYTES];
	UINT8 version[SIZE_VERSION_IN_BYTES];
	UINT8 header[HEADER_SIZE_BYTES];
	UINT32 dwRes, header_offset=0;
	UINT32 nread = 0;
	UINT32 i=0, j=0;
	UINT32 last_block;
	UINT64 size, num_blocks;
	Keccak_DuplexInstance spongeWrap;

	//first, get the size of the encrypted file and
	TRY
	{
		//get the file size
		size = pfIN->GetLength();
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//the file should be large enough to hold the overhead 
	if( size < SIZE_OVERHEAD_IN_BYTES )
	{
		return ERROR_INVALID_FILE;
	}	
	//read the mac from the encrypted file
	TRY
	{
		//set file pointer to the beginning of the mac
		pfIN->Seek(-SIZE_MAC_IN_BYTES, CFile::end );
		//read the mac
		nread = pfIN->Read(mac_from_file, SIZE_MAC_IN_BYTES);
		//reset the file pointer
		pfIN->SeekToBegin();
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

		//get the real number of bytes to be decrypted
		num_bytes = (UINT32)size - SIZE_OVERHEAD_IN_BYTES;

	//we should have read enough bytes to hold a valid mac
	if( nread < SIZE_MAC_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//derive the number of full key stream blocks required and the size of the
	//the last incomplete key stream block (could be zero)
	num_blocks = (size - SIZE_OVERHEAD_IN_BYTES)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = ((UINT32)size - SIZE_OVERHEAD_IN_BYTES)%(UINT32)KECCAK_IO_SIZE_BYTES;

	if(0 == last_block)
	{
		if(0 == num_blocks)
		{
			return ERROR_NONE;
		}
		else
		{
			num_blocks--;
			last_block = KECCAK_IO_SIZE_BYTES;
		}
	}

	//try to read the first bytes which should hold the fixed marker
	TRY
	{
		nread = pfIN->Read(mark, SIZE_MARK_IN_BYTES);
		memcpy(header, mark, SIZE_MARK_IN_BYTES);
		header_offset += SIZE_MARK_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we should have read enough bytes to hold a valid marker
	if( nread < SIZE_MARK_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//check the read marker against the fixed marker
	if( memcmp(mark, MARK_ENCNOTEBOOK, SIZE_MARK_IN_BYTES) )
	{
		return ERROR_INVALID_FILE;
	}

	//try to read the version
	TRY
	{
		nread = pfIN->Read(version, SIZE_VERSION_IN_BYTES);
		memcpy(header + header_offset, version, SIZE_VERSION_IN_BYTES);
		header_offset += SIZE_VERSION_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we should have read enough bytes to hold a valid marker
	if( nread < SIZE_VERSION_IN_BYTES )
	{
		return ERROR_READ_FROM_FILE;
	}

	//check the read marker against the fixed marker
	dwRes = _parse_version(DATATYPE_NOTEBOOK, version);

	if( ERROR_NONE != dwRes )
	{
		return dwRes;
	}

	//try to read the iv from file
	TRY
	{
		nread = pfIN->Read(iv, SIZE_IV_IN_BYTES);
		memcpy(header + header_offset, iv, SIZE_IV_IN_BYTES);
		header_offset += SIZE_IV_IN_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we really should be getting enough bytes for the iv
	if( nread != SIZE_IV_IN_BYTES)
	{
		return ERROR_READ_FROM_FILE;
	}

	//try to read the first bytes which should hold the password check
	TRY
	{
		nread = pfIN->Read(check_from_file, PSWD_CHECK_SIZE_BYTES);
		memcpy(header + header_offset, check_from_file, PSWD_CHECK_SIZE_BYTES);
		header_offset += PSWD_CHECK_SIZE_BYTES;
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we really should be getting enough bytes for the pswd check
	if( nread != PSWD_CHECK_SIZE_BYTES)
	{
		return ERROR_READ_FROM_FILE;
	}

	//we have the password and the iv, so set up keccak
	if( _set_up_crypto(&spongeWrap, pstrPassword, iv, check ) )
	{
		return ERROR_CRYPTO;
	}

	//check the read marker against the fixed marker
	if( memcmp(check, check_from_file, PSWD_CHECK_SIZE_BYTES) )
	{
		return ERROR_INCORRECT_PSWD;
	}

	//generate first block of key stream
	if( Keccak_Duplexing(&spongeWrap, header, header_offset, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		return ERROR_CRYPTO;
	}

	//loop over the full key stream blocks
	for(i=0; i<num_blocks; i++)
	{
		//try to read a full output block
		TRY
		{
			nread = pfIN->Read(buf_in, KECCAK_IO_SIZE_BYTES);
		}
		CATCH(CFileException, e){

			//something went wrong when reading from the file 
			return ERROR_READ_FROM_FILE;
		}
		END_CATCH

		//we should have a full block 
		if(KECCAK_IO_SIZE_BYTES != nread)
		{
			return ERROR_READ_FROM_FILE;
		}	

		//xor the key stream bytes with the bytes read from the file
		for(j=0; j<nread; j++)
		{
			buf[j + i*KECCAK_IO_SIZE_BYTES] = (buf_in[j] ^ ks[j]);
		}

		//create new key stream
		if( Keccak_Duplexing(&spongeWrap, buf + i*KECCAK_IO_SIZE_BYTES, nread, ks, KECCAK_IO_SIZE_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ONE) )
		{
			return ERROR_CRYPTO;
		}
	}

	TRY
	{
		nread = pfIN->Read(buf_in, last_block);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	//we know how much we should have read
	if(last_block != nread)
	{
		return ERROR_READ_FROM_FILE;
	}

	//xor the key stream bytes with the bytes read from the file
	for(i=0; i<nread; i++)
	{
		buf[i + num_blocks*KECCAK_IO_SIZE_BYTES] = (buf_in[i] ^ ks[i]);
	}

	//create new mac
	if( Keccak_Duplexing(&spongeWrap, buf + num_blocks*KECCAK_IO_SIZE_BYTES, last_block, mac, SIZE_MAC_IN_BYTES, KECCAK_SIGMAEND_TRAIL_BIT_ZERO) )
	{
		return ERROR_CRYPTO;
	}

	//compare the computed mac with the mac from file
	if(!memcmp(mac, mac_from_file, SIZE_MAC_IN_BYTES))
	{
		return ERROR_NONE;
	}
	else
	{
		return ERROR_CORRUPT_FILE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with managing password notes
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	add a password note to the loaded notebook
*/
BOOL CCore::AddPassword( note_password* pPassword )
{
	if(NULL == pPassword)
	{
		return FALSE;
	}
	ASSERT(pPassword != NULL);
	int i,n;
	
	//compute and add the checksum
	_set_data_from_password(pPassword);
	//set the size fields of the password note
	_set_sizes(pPassword);

	CPasswordNotesArray* pNotes = GetPasswordArray( pPassword->dwType );

	//assure we have a notebook loaded
	if(NULL == pNotes)
	{
		return FALSE;
	}
	ASSERT(pNotes != NULL);

	//find the size of the notebook
	n = pNotes->GetSize();

	//if the notebook is ridiculously large, bail out
	if(n >= MAX_NUM_ENTRIES_NOTEBOOK)
	{
		ReportError(ERROR_NOTEBOOK_FULL);
	}

	//check if the password is already there
	for(i=0; i<n; i++)
	{
		//already there?
		if(  pNotes->GetAt(i).strDomain == pPassword->strDomain &&
			 pNotes->GetAt(i).strInfo == pPassword->strInfo &&
			 pNotes->GetAt(i).strPassword == pPassword->strPassword )
		{
			return FALSE;
		}
	}

	//add the password to the linked list
	pNotes->Add( *pPassword );
	
	//indicate the notebook has changed
	m_notebook.changed_after_loading= TRUE;

	return TRUE;
}

/*
	remove a password from the notebook
*/
BOOL CCore::RemovePassword(note_password* pPswd)
{
	int i,n;
	UINT32 type;
	CPasswordNotesArray* pNotes;

	//loop over the different types of password notes
	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray(type);

		//if we can not access the linked list, something is wrong
		if(NULL == pNotes)
		{
			return FALSE;
		}
		ASSERT(pNotes != NULL);

		//get the size of the linked list  
		n = pNotes->GetSize();

		//walk down the list in backward direction
		for(i=n-1; i>=0; i--)
		{
			//find the password
			if( (pPswd->strDate == pNotes->GetAt(i).strDate) &&
				(pPswd->strPassword == pNotes->GetAt(i).strPassword) &&
				(pPswd->strInfo == pNotes->GetAt(i).strInfo &&
				(pPswd->strDomain == pNotes->GetAt(i).strDomain)))
			{
				//clear the content of the password note
				ClearString( &(pNotes->GetAt(i).strPassword) );
				ClearString( &(pNotes->GetAt(i).strInfo) );
				ClearString( &(pNotes->GetAt(i).strDomain) );
				ClearString( &(pNotes->GetAt(i).strDate) );

				//remove it from the linked list
				pNotes->RemoveAt(i);

				//indicate our notebook has changed
				m_notebook.changed_after_loading= TRUE;

				return TRUE;
			}
		}
	}

	//done
	return TRUE;
}

/*
	clean the password
*/
void CCore::CleanPassword( t_note_password* pPswd )
{
	ASSERT(pPswd != NULL);

	//clear the content
	ClearString(&pPswd->strPassword);
	ClearString(&pPswd->strDate);
	ClearString(&pPswd->strDomain);
	ClearString(&pPswd->strInfo);

	//reset the integer fields
	pPswd->dwData =0;
	pPswd->dwSize =0;
	pPswd->dwStrDateByteLength =0;
	pPswd->dwStrDomainByteLength =0;
	pPswd->dwStrInfoByteLength =0;
	pPswd->dwStrPasswordByteLength =0;
	pPswd->dwType =0;

	//set the iv/salt to zero
	memset(pPswd->iv, 0, SIZE_IV_IN_BYTES);
}

/*
	complete and add a file password note to the loaded notebook
*/
BOOL CCore::_complete_and_add_file_password( CString* pstrPassword, CString strURL, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	ASSERT(pstrPassword != NULL);

	t_note_password pswd;
	BOOL fRes;

	//trunc the filepath to just the filename
	CString strFileName = ::PathFindFileNameW(strURL);

	//fill in the gaps
	_complete_file_password(&pswd, pstrPassword, strFileName, iv );

	//add it to the notebook
	fRes = AddPassword(&pswd);

	CleanPassword(&pswd);

	return fRes;
}

/*
	complete a password of type file
*/
void CCore::_complete_file_password( t_note_password* pPswd, CString* pstrPassword, CString strURL, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	ASSERT(pPswd != NULL);
	ASSERT(pstrPassword != NULL);

	//add the current date to the password
	CString strDate;
	strDate = GetCurrentDateAsString();
	pPswd->strDate = strDate;
	//set the password
	pPswd->strPassword = *pstrPassword;
	//set the domain info to be void
	pPswd->strDomain = strURL;
	//set the filename into the info string
	pPswd->strInfo = _T("");
	//set the password type to be 'FILE'
	pPswd->dwType = PSWD_FILE;
	//set the iv of the encrypted file
	memcpy(pPswd->iv, iv, SIZE_IV_IN_BYTES);
}

/*
	compute and set the sizes of the fields in the password note
*/
void CCore::_set_sizes( note_password* pPassword )
{
	if(NULL == pPassword)
	{
		return;
	}
	ASSERT(pPassword != NULL);

	//get the string lengths
	pPassword->dwStrInfoByteLength = pPassword->strInfo.GetLength();
	pPassword->dwStrDomainByteLength = pPassword->strDomain.GetLength();
	pPassword->dwStrPasswordByteLength = pPassword->strPassword.GetLength();
	pPassword->dwStrDateByteLength = pPassword->strDate.GetLength();

	//set the raw size of the password entry on disc 
	pPassword->dwSize = 28 + SIZE_IV_IN_BYTES;
	pPassword->dwSize += (sizeof(wchar_t)*(pPassword->dwStrInfoByteLength));
	pPassword->dwSize += (sizeof(wchar_t)*(pPassword->dwStrDomainByteLength));
	pPassword->dwSize += (sizeof(wchar_t)*(pPassword->dwStrPasswordByteLength));
	pPassword->dwSize += (sizeof(wchar_t)*(pPassword->dwStrDateByteLength));
}

/*
	compute and set the checksum over the password note content
*/
void CCore::_set_data_from_password( note_password* pPassword )
{
	if(NULL == pPassword)
	{
		return;
	}
	ASSERT(pPassword != NULL);

	pPassword->dwData = _compute_checksum( pPassword );
}

/*
	compute the checksum over the password note content
*/
UINT32 CCore::_compute_checksum( t_note_password *pPswd )
{
	if(NULL == pPswd)
	{
		return 0;
	}
	ASSERT(pPswd != NULL);
	Keccak_DuplexInstance sponge;
	UINT8* buffer;
	UINT8 data[4];
	UINT32 dwCheck =0;
	int index=0, len=0;
	int i, num_blocks=0, last_block=0;

	//initialize the keccak duplex object
	Keccak_DuplexInitialize(&sponge, KECCAK_RATE_BITS, KECCAK_CAPACITY_BITS);

	//compute the size of the password content
	len =  sizeof(wchar_t)*pPswd->strDate.GetLength();
	len += sizeof(wchar_t)*pPswd->strDomain.GetLength();
	len += sizeof(wchar_t)*pPswd->strInfo.GetLength();
	len += sizeof(wchar_t)*pPswd->strPassword.GetLength();
	len += SIZE_IV_IN_BYTES;

	//initialize the buffer that will hold the password content
	buffer = new UINT8[len];

	//deduce the number of blocks to absorb and the size of the last block
	num_blocks = (len)/((UINT64)KECCAK_IO_SIZE_BYTES);
	last_block = (len)%(UINT64)KECCAK_IO_SIZE_BYTES;

	if(0 == last_block)
	{
		num_blocks--;
		last_block = KECCAK_IO_SIZE_BYTES;
	}

	//fill the buffer with the password content
	memcpy(buffer,  pPswd->strDate.GetBuffer(), sizeof(wchar_t)*pPswd->strDate.GetLength());
	index += sizeof(wchar_t)*pPswd->strDate.GetLength();
	memcpy(buffer+index,  pPswd->strDomain.GetBuffer(), sizeof(wchar_t)*pPswd->strDomain.GetLength());
	index += sizeof(wchar_t)*pPswd->strDomain.GetLength();
	memcpy(buffer+index,  pPswd->strInfo.GetBuffer(), sizeof(wchar_t)*pPswd->strInfo.GetLength());
	index += sizeof(wchar_t)*pPswd->strInfo.GetLength();
	memcpy(buffer+index,  pPswd->strPassword.GetBuffer(), sizeof(wchar_t)*pPswd->strPassword.GetLength());
	index += sizeof(wchar_t)*pPswd->strPassword.GetLength();
	memcpy(buffer+index, pPswd->iv, SIZE_IV_IN_BYTES);

	//absorb the buffer holding all the data
	for(i=0; i<num_blocks; i++)
	{
		Keccak_Duplexing(&sponge, buffer + i*KECCAK_IO_SIZE_BYTES, KECCAK_IO_SIZE_BYTES, NULL, 0, KECCAK_SIGMAEND_TRAIL_BIT_ONE);
	}

	//absorb the last block
	Keccak_Duplexing(&sponge, buffer + num_blocks*KECCAK_IO_SIZE_BYTES, last_block, data, 4, KECCAK_SIGMAEND_TRAIL_BIT_ZERO);

	//load the checksum
	dwCheck = data[0];
	dwCheck <<= 8;
	dwCheck |= data[1];
	dwCheck <<= 8;
	dwCheck |= data[2];
	dwCheck <<= 8;
	dwCheck |= data[3];

	//clean and delete the buffer
	SecureZeroMemory(buffer, len);
	delete[] buffer;

	return dwCheck;
}

/*
	check a password note to contain a correct checksum
*/
BOOL CCore::_check_password( t_note_password* pPswd )
{
	if(NULL == pPswd)
	{
		return FALSE;
	}
	ASSERT(pPswd != NULL);

	return (pPswd->dwData == _compute_checksum(pPswd) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stuff to do with managing the notebook
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CString CCore::GetTemporaryFileName( CString strTarget )
{
	CString strTemp = strTarget;
	wchar_t szTempName[MAX_PATH];

	::PathRemoveFileSpecW(strTemp.GetBuffer((0)));
	strTemp.ReleaseBuffer(-1);

	::GetTempFileName(strTemp, _T("ktf"), 0, szTempName);
	strTemp = (LPWSTR) szTempName;

	return strTemp;
}

CString CCore::GetCurrentDateAsString()
{
	CTime now = CTime::GetCurrentTime();
	CString strNow;
	strNow.Format( _T("%4d-%02d-%02d"), now.GetYear(), now.GetMonth(), now.GetDay() );

	return strNow;
}



UINT32 CCore::_write_notebook_to_file()
{
	UINT32 index =0;
	UINT32 res, type;
	int i,n;
	note_password pswd;
	CFile f_out;
	CFileException FE;
	CString strTempName;
	CPasswordNotesArray *pNotes;

	//if no notebook is loaded, there is nothing to save
	if(FALSE == m_fNotebookLoaded)
	{
		return ERROR_NO_NOTEBOOK_LOADED;
	}

	//if the notebook has not changed, there is no reason to save
	if(FALSE == m_notebook.changed_after_loading)
	{
		return ERROR_NOTEBOOK_UNCHANGED;
	}

	UINT8* pLargeBuf = new BYTE[MAX_SIZE_NOTEBOOK_BYTES];

	//compute the size of the notebook we wish to write to file
	res = _compute_notebook_size_in_bytes();

	//if we could not determine the size, bail out
	if(ERROR_NONE != res)
	{
		return res;
	}

	//write the total size of the notebook into the buffer
	memcpy(pLargeBuf + index, &m_notebook.total_byte_size, 4);
	index += 4;

	//compute the total number of entries in the notebook
	res = _compute_num_entries_notebook();

	//if we could not determine the number of entries, bail out
	if(ERROR_NONE != res)
	{
		return res;
	}

	//write the total number of entries into the buffer
	memcpy(pLargeBuf + index, &m_notebook.num_entries, 4);
	index += 4;

	//now, loop over all passwords and write each into the buffer
	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray(type);
		n = pNotes->GetSize();

		for(i=0; i<n; i++)
		{
			pswd = pNotes->GetAt(i);

			memcpy(pLargeBuf + index, &pswd.dwSize, 4);
			index += 4;
			memcpy(pLargeBuf + index, &pswd.dwType, 4);
			index += 4;
			memcpy(pLargeBuf + index, &pswd.dwData, 4);
			index += 4;

			memcpy(pLargeBuf + index, &pswd.dwStrDateByteLength, 4);
			index += 4;
			memcpy(pLargeBuf + index, pswd.strDate.GetBuffer(), sizeof(wchar_t)*pswd.strDate.GetLength());
			index += sizeof(wchar_t)*pswd.strDate.GetLength();

			memcpy(pLargeBuf + index, &pswd.dwStrDomainByteLength, 4);
			index += 4;
			memcpy(pLargeBuf + index, pswd.strDomain.GetBuffer(), sizeof(wchar_t)*pswd.strDomain.GetLength());
			index += sizeof(wchar_t)*pswd.strDomain.GetLength();

			memcpy(pLargeBuf + index, &pswd.dwStrInfoByteLength, 4);
			index += 4;
			memcpy(pLargeBuf + index, pswd.strInfo.GetBuffer(), sizeof(wchar_t)*pswd.strInfo.GetLength());
			index += sizeof(wchar_t)*pswd.strInfo.GetLength();

			memcpy(pLargeBuf + index, &pswd.dwStrPasswordByteLength, 4);
			index += 4;
			memcpy(pLargeBuf + index, pswd.strPassword.GetBuffer(), sizeof(wchar_t)*pswd.strPassword.GetLength());
			index += sizeof(wchar_t)*pswd.strPassword.GetLength();

			memcpy(pLargeBuf + index, pswd.iv, SIZE_IV_IN_BYTES);
			index += SIZE_IV_IN_BYTES;
		}
	}

	strTempName = GetTemporaryFileName(m_notebook.strURL);

	//now we have all the data in the buffer, try to open the output file
	TRY
	{
		f_out.Open(strTempName, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when writing to the file 

		//clean up the buffer first
		SecureZeroMemory(pLargeBuf, index);
		delete[] pLargeBuf;

		//bail out
		return ERROR_WRITE_TO_FILE;
	}
	END_CATCH


	//encrypt the buffer and write to file
	res = _encrypt_buffer_to_file( &m_notebook.strPassword, pLargeBuf, index, &f_out );

	//clean up the buffer that held the sensitive data
	SecureZeroMemory(pLargeBuf, index);
	delete[] pLargeBuf;

	
	//close the files
	f_out.Flush();
	f_out.Close();

	//if encryption of the notebook went well, rename the temp file
	if(ERROR_NONE == res)
	{
		//if the target file already exists, remove it
		TRY 
		{
			CFile::Remove(m_notebook.strURL);
		}
		CATCH (CFileException, e)
		{
			//the target file did not exist,
			//so just continue
		}
		END_CATCH

		//do the renaming 
		TRY 
		{
			CFile::Rename(strTempName, m_notebook.strURL);
			SaveLastUsedNotebookURLToRegistry(m_notebook.strURL);
		}
		CATCH (CFileException, e)
		{
			//something went wrong when writing to the file 
			//try to delete the temp file			
			TRY 
			{
				CFile::Remove(strTempName);
			}
			CATCH (CFileException, e)
			{
				//we are out of options
				//so just return with the error code
				res = ERROR_WRITE_TO_FILE;
			}
			END_CATCH

			res = ERROR_WRITE_TO_FILE;
		}
		END_CATCH
	}
	//if here was an error on decryption, clean up the temp file
	else
	{
		TRY 
		{
			CFile::Remove(strTempName);
		}
		CATCH (CFileException, e)
		{
			//we are out of options
			//so just return with the error code
		}
		END_CATCH
	}

	return res;
}






UINT32 CCore::_parse_notebook_from_file( CString strURL, CString* pstrPassword )
{
	ASSERT(pstrPassword != NULL);

	UINT32 res = ERROR_NONE;
	CFile f_in;
	CFileException FE;
	UINT32 num_bytes;
	
	TRY
	{
		f_in.Open( strURL,  CFile::modeRead|CFile::shareExclusive, &FE);
	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		return ERROR_READ_FROM_FILE;
	}
	END_CATCH

	UINT8* pLargeBuf = new BYTE[MAX_SIZE_NOTEBOOK_BYTES];

	res = _decrypt_file_into_buffer( pstrPassword, pLargeBuf, num_bytes, &f_in );

	f_in.Close();

	if(ERROR_NONE == res)
	{
		res = _parse_notebook_from_buffer( pLargeBuf, num_bytes);

		if(ERROR_NONE == res)
		{
			SetNewNotebookURL( strURL );
			SetNewNotebookPassword( pstrPassword );

			m_notebook.changed_after_loading = FALSE;

			SaveLastUsedNotebookURLToRegistry(strURL);
		}
		else
		{
			CleanNotebook();
		}
	}

	delete[] pLargeBuf;

	ClearString( pstrPassword );

	return res;
}


int CCore::_parse_notebook_from_buffer( UINT8* buf, UINT32 size )
{

	UINT32 index=0;
	UINT32 size_from_buffer=0;
	UINT32 num_entries=0;
	UINT32 i=0;
	t_note_password pswd;
	wchar_t* tmp;

	memcpy(&size_from_buffer, buf+index, sizeof(UINT32));
	index += sizeof(UINT32);

	if( (index > size) || (size_from_buffer + 8 != size) || (size_from_buffer > MAX_SIZE_NOTEBOOK_BYTES) )
	{
		return ERROR_CORRUPT_FILE;
	}

	memcpy(&num_entries, buf+index, sizeof(UINT32));
	index += sizeof(UINT32);

	if( (index > size) || (num_entries > MAX_NUM_ENTRIES_NOTEBOOK) )
	{
		return ERROR_CORRUPT_FILE;
	}

	for(i=0; i<num_entries; i++)
	{
		memcpy(&pswd.dwSize, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if( (index > size) || (pswd.dwSize > MAX_SIZE_NOTE_BYTES))
		{
			return ERROR_CORRUPT_FILE;
		}

		memcpy(&pswd.dwType, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if( (index > size) || (pswd.dwType > (NUM_PASSWORD_TYPES-1) ))
		{
			return ERROR_CORRUPT_FILE;
		}

		memcpy(&pswd.dwData, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if(index > size)
		{
			return ERROR_CORRUPT_FILE;
		}

		memcpy(&pswd.dwStrDateByteLength, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if( (index > size) || (pswd.dwStrDateByteLength != 10) )
		{
			return ERROR_CORRUPT_FILE;
		}

		tmp = new wchar_t[pswd.dwStrDateByteLength];
		memcpy(tmp, buf+index, (sizeof(wchar_t))*pswd.dwStrDateByteLength);
		index += (sizeof(wchar_t))*pswd.dwStrDateByteLength;

		if(index > size)
		{
			return ERROR_CORRUPT_FILE;
		}

		pswd.strDate = CString(tmp);
		pswd.strDate = pswd.strDate.Left(pswd.dwStrDateByteLength);
		delete[] tmp;

		
		memcpy(&pswd.dwStrDomainByteLength, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if( (index + pswd.dwStrDomainByteLength > size) || (pswd.dwStrDomainByteLength > MAX_PSWD_SIZE) )
		{
			return ERROR_CORRUPT_FILE;
		}

		tmp = new wchar_t[pswd.dwStrDomainByteLength];
		memcpy(tmp, buf+index, pswd.dwStrDomainByteLength*sizeof(wchar_t));
		index += pswd.dwStrDomainByteLength*sizeof(wchar_t);

		if(index > size)
		{
			return ERROR_CORRUPT_FILE;
		}

		pswd.strDomain = CString(tmp);
		pswd.strDomain = pswd.strDomain.Left(pswd.dwStrDomainByteLength);
		delete[] tmp;

		memcpy(&pswd.dwStrInfoByteLength, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if((index + pswd.dwStrInfoByteLength> size) || (pswd.dwStrInfoByteLength > MAX_PSWD_SIZE))
		{
			return ERROR_CORRUPT_FILE;
		}

		tmp = new wchar_t[pswd.dwStrInfoByteLength];
		memcpy(tmp, buf+index, pswd.dwStrInfoByteLength*sizeof(wchar_t));
		index += pswd.dwStrInfoByteLength*sizeof(wchar_t);

		if(index > size)
		{
			return ERROR_CORRUPT_FILE;
		}

		pswd.strInfo = CString(tmp);
		pswd.strInfo = pswd.strInfo.Left(pswd.dwStrInfoByteLength);
		delete[] tmp;

		memcpy(&pswd.dwStrPasswordByteLength, buf+index, sizeof(UINT32));
		index += sizeof(UINT32);

		if((index + pswd.dwStrPasswordByteLength> size) || (pswd.dwStrPasswordByteLength > MAX_PSWD_SIZE))
		{
			return ERROR_CORRUPT_FILE;
		}

		tmp = new wchar_t[pswd.dwStrPasswordByteLength];
		memcpy(tmp, buf+index, pswd.dwStrPasswordByteLength*sizeof(wchar_t));
		index += pswd.dwStrPasswordByteLength*sizeof(wchar_t);

		if( index + SIZE_IV_IN_BYTES > size )
		{
			return ERROR_CORRUPT_FILE;
		}

		pswd.strPassword = CString(tmp);
		pswd.strPassword = pswd.strPassword.Left(pswd.dwStrPasswordByteLength);
		SecureZeroMemory(tmp, pswd.dwStrPasswordByteLength*sizeof(wchar_t));
		delete[] tmp;

		memcpy(pswd.iv, buf+index, SIZE_IV_IN_BYTES);
		index += SIZE_IV_IN_BYTES;

		if( _check_password(&pswd) )
		{
			AddPassword(&pswd);
		}
		else
		{
			return ERROR_CORRUPT_FILE;
		}
	}

	return ERROR_NONE;
}


UINT32 CCore::_compute_num_entries_notebook()
{
	UINT32 type;
	UINT32 num=0;
	CPasswordNotesArray* pNotes = NULL;

	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray( type );

		if(NULL == pNotes)
		{
			return ERROR_NO_NOTEBOOK_LOADED;
		}
		ASSERT(pNotes != NULL);

		num += pNotes->GetSize();
	}
	
	if(num > MAX_NUM_ENTRIES_NOTEBOOK)
	{
		return ERROR_NOTEBOOK_TOO_LARGE;
	}
	else
	{
		m_notebook.num_entries = num;
		return ERROR_NONE;
	}
}

UINT32 CCore::_compute_notebook_size_in_bytes()
{
	UINT32 i,n=0;
	m_notebook.total_byte_size = 0;
	t_note_password* pswd;
	UINT32 type;
	CPasswordNotesArray* pNotes;

	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray( type );

		if(NULL == pNotes)
		{
			return ERROR_NO_NOTEBOOK_LOADED;
		}
		ASSERT(pNotes != NULL);

		n = pNotes->GetSize();

		if(n > MAX_NUM_ENTRIES_NOTEBOOK)
		{
			return ERROR_NOTEBOOK_TOO_LARGE;
		}

		for(i=0; i<n; i++)
		{
			pswd = &(pNotes->GetAt(i));

			if(pswd->dwSize > MAX_SIZE_NOTE_BYTES)
			{
				return ERROR_ENTRY_TOO_LARGE;
			}

			m_notebook.total_byte_size += pswd->dwSize;

		}
	}

	return ERROR_NONE;
}










CPasswordNotesArray* CCore::GetPasswordArray( UINT32 dwType )
{
	switch( dwType )
	{
		case PSWD_ACCOUNT:
			return &m_notebook.passwords_accounts;
		case PSWD_CONTACT:
			return &m_notebook.passwords_contacts;
		case PSWD_FILE:
			return &m_notebook.passwords_files;
		case PSWD_MESSAGE:
			return &m_notebook.passwords_messages;
		default:
			return NULL;
	}
}

BOOL CCore::AskAndLoadNotebook(CString strStartURL)
{
	CWelcomeDlg welcome;
	CString strPassword, strURL;
	BOOL fDone = FALSE;
	BOOL fRes = FALSE;
	UINT32 res;

	if(FALSE == ::PathFileExistsW(strStartURL))
	{
		return FALSE;
	}

	welcome.SetLastUsedNotebookURL( strStartURL );
	welcome.attach(this);

	do 
	{
		if( IDOK == welcome.DoModal() )
		{
			strPassword = welcome.GetPassword();
			res = StartLoadNotebook( strStartURL, &strPassword );
			ClearString(&strPassword);

			if(ERROR_NONE == res)
			{
				SetNotebookLoadedFlag(TRUE);
				fDone = TRUE;
				fRes = TRUE;
			}
			else 
			{
				ReportError(res);
			}
		}
		else
		{
			fRes = FALSE;
			fDone = TRUE;
		}

	} while( !fDone );


	return fRes;
}

BOOL CCore::GetLastUsedNotebookURL(CString &strURL)
{
	CString strTmp;
	
	//look in the registry to find the last used notebook url
	if(FALSE == RetrieveLastUsedNotebookURLToRegistry(strTmp) )
	{
		return FALSE;
	}

	//check if the recovered url still exists
	if(::PathFileExistsW(strTmp))
	{
		strURL = strTmp;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCore::SelectNotebookURL(CString &strURL)
{
	CString strFilter;
	BOOL fDone  = FALSE;

	//build filter for file open dialog
	strFilter.Format(_T("%s (*.%s)|*.%s||"), _T("Krypcak Notebook"), _T("knb"), _T("knb"));


	do{
		
		CFileDialog dlgNotebook(true, NULL, NULL, OFN_HIDEREADONLY, strFilter);	
		dlgNotebook.m_ofn.lpstrTitle = _T("Select notebook");


		//if the user has not selected a plain file, we are done
		if( IDOK != dlgNotebook.DoModal())
		{
			//clear the member holding the path of the file to be encrypted
			return FALSE; 
		}
		else 
		{
			//get the selected url
			strURL = dlgNotebook.GetPathName();
	
			//check if the selected url exists
			if( ::PathFileExistsW( strURL ))
			{
				fDone  = TRUE;
			}
			else
			{
				AfxMessageBox( IDS_ERR_NONEXIST_NB );
			}
		}

	}while(!fDone);

	
	return TRUE;
}

void CCore::CloseAndCleanUp()
{
	//close and save the notebook
	if(m_fNotebookLoaded)
	{
		//save the notebook to file
		if(m_notebook.changed_after_loading)
		{
			StartSaveNotebook();
		}
		
		//clean the notebook in memory
		CleanNotebook();

		//set the flag to indicate no notebook is present anymore
		m_fNotebookLoaded = FALSE;
	}
}

void CCore::CleanNotebook()
{
	int i,n;
	UINT32 type;
	CPasswordNotesArray* pNotes;

	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray(type);
		
		if(NULL == pNotes)
		{
			return;
		}
		ASSERT(pNotes != NULL);

		n = pNotes->GetSize();

		for(i=n-1; i>=0; i--)
		{
			ClearString( &(pNotes->GetAt(i).strPassword) );
			ClearString( &(pNotes->GetAt(i).strInfo) );
			ClearString( &(pNotes->GetAt(i).strDomain) );
			ClearString( &(pNotes->GetAt(i).strDate) );

			pNotes->RemoveAt(i);
		}

	}
	
	ClearString(&m_notebook.strPassword);
	ClearString(&m_notebook.strURL);
	m_notebook.num_entries = 0;
	
	SetNotebookLoadedFlag(FALSE);
}

void CCore::CopyStringToClipBoard(CString* pstrData, BOOL fWipe)
{
	HGLOBAL h;
	LPWSTR arr;

	//copy the string into 
	size_t bytes = (pstrData->GetLength()+1)*sizeof(wchar_t);
	h = GlobalAlloc(GMEM_MOVEABLE, bytes);
	arr = (LPWSTR)GlobalLock(h);
	ZeroMemory(arr, bytes);
	_tcscpy_s(arr, pstrData->GetLength()+1, *pstrData);
	pstrData->ReleaseBuffer();
	GlobalUnlock(h);

	//open the clipboard 
	::OpenClipboard(NULL);
	//clear the clipboard
	EmptyClipboard();
	//copy the string to the clipboard 
	SetClipboardData(CF_UNICODETEXT, h);
	//close the clipboard
	CloseClipboard();

	//start thread to clear the clipboard after a given time
	if(fWipe)
	{
		_beginthread( DelayedEmptyClipBoard, 0, NULL);
	}
	
}

void CCore::DelayedEmptyClipBoard( void* )
{
	//sleep
	Sleep(NUM_SECS_PSWD_ON_CLIPBOARD*1000);

	//open the clipboard
	::OpenClipboard(NULL);
	//clear the clipboard
	EmptyClipboard();
	//close the clipboard
	CloseClipboard();
}



void CCore::SaveLastUsedNotebookURLToRegistry( CString strURL )
{
	CRegKey		RegKey;
	HKEY		hKey( HKEY_CURRENT_USER );

	//try to access the krypcak\\last used paths entry in the registry 
	if ( RegKey.Open( hKey, _T("SOFTWARE\\Krypcak\\LastUsedPaths"), KEY_ALL_ACCESS) != ERROR_SUCCESS)
	{
		// couldn't open the key, try creating it..
		if ( RegKey.Create( hKey, _T("SOFTWARE\\Krypcak\\LastUsedPaths"), REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL ) != S_OK )

			// failed, bail out..
			return;
	}

	//try to set the last used path for the notebook
	if ( RegKey.SetStringValue( _T("Notebook") , strURL )
		!= ERROR_SUCCESS )
	{
		return;
	}

	RegKey.Close();	
}

BOOL CCore::RetrieveLastUsedNotebookURLToRegistry( CString &strURL )
{
	CRegKey		RegKey;
	HKEY		hKey( HKEY_CURRENT_USER );
	ULONG		len = 255; //maximum size

	//try to access the krypcak\\last used paths entry in the registry 
	if ( RegKey.Open( hKey, _T("SOFTWARE\\Krypcak\\LastUsedPaths"), KEY_ALL_ACCESS) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegKey.QueryStringValue( _T("Notebook"), strURL.GetBufferSetLength( len ),  &len );
	strURL.ReleaseBuffer();

	RegKey.Close();	

	return TRUE;
}

void CCore::ChangePasswordNotebook()
{
	CSetPasswordDlg dlg;
	CString strPassword;
	BOOL fDone = FALSE;
	UINT32 res;

	if(FALSE == m_fNotebookLoaded)
	{
		return;
	}

	//configure the password dialog
	dlg.attach(this);
	dlg.SetConfirm(FALSE);
	dlg.SetEnableStorePassword(FALSE);
	dlg.SetEnableGenerateButton(FALSE);
	dlg.SetEnableLoadButton(FALSE);
	dlg.SetShowStorePasswordCheck(FALSE);
	dlg.SetHeaderText(_T("Enter current password"));

	//ask for the current password
	do
	{
			//launch the password dialog
			res = dlg.DoModal();

			if(res != IDOK)
			{
				//if the user did not press ok, we are done
				fDone = TRUE;
				return;
			}
			else
			{
				//compare with the sored password
				if( m_notebook.strPassword == dlg.GetPassword() )
				{
					//match, so ok
					fDone = TRUE;
				}
				else
				{
					//no match
					ReportError(ERROR_INCORRECT_PSWD);
				}
			}

	}while(!fDone);

	//configure the password dialog
	dlg.attach(this);
	dlg.SetConfirm(TRUE);
	dlg.SetEnableStorePassword(FALSE);
	dlg.SetEnableGenerateButton(TRUE);
	dlg.SetEnableLoadButton(FALSE);
	dlg.SetShowStorePasswordCheck(FALSE);
	dlg.SetHeaderText(_T("Enter new password"));

	//ask for the new password
	if( IDOK == dlg.DoModal() )
	{
		//set the new password
		strPassword = dlg.GetPassword();
		SetNewNotebookPassword( &strPassword );
		AfxMessageBox( IDS_PSWD_CHANGED, MB_ICONINFORMATION );

		ClearString( &strPassword );
	}
}

BOOL CCore::SetNewNotebookURL( CString strURL )
{
	//TODO check if URL is writable
	m_notebook.strURL = strURL;
	m_notebook.changed_after_loading =TRUE;

	return TRUE;
}

BOOL CCore::SetNewNotebookPassword( CString* pstrPassword )
{
	ASSERT(pstrPassword != NULL);

	m_notebook.strPassword = *pstrPassword;
	m_notebook.changed_after_loading =TRUE;

	return TRUE;
}


CString CCore::GetLoadedNotebookURL()
{
	return m_notebook.strURL;
}

/*
	set the flag indicating that a notebook has been loaded
*/
void CCore::SetNotebookLoadedFlag( BOOL flag )
{
	m_fNotebookLoaded = flag;
}

/*
	remove the .kef extension from the name of an encrypted file
*/
void CCore::StripDecryptFileName( CString &strURL )
{
	UINT32 len = strURL.GetLength();

	if( strURL.Right( ENC_FILE_EXT.GetLength() ) == ENC_FILE_EXT)
	{
		strURL= strURL.Left( len-ENC_FILE_EXT.GetLength() );
	}
}

/*
	get the user to select an url for a new notebook
*/
BOOL CCore::SelectNewNotebookURL( CString *pstrURL )
{
	ASSERT(pstrURL != NULL);
	
	CString strFilter;

	//build filter for file open dialog
	strFilter.Format(_T("%s (*.%s)|*.%s||"), _T("Krypcak Notebook"), _T("knb"), _T("knb"));

	CFileDialog dlgSave(true, NULL, NULL, OFN_HIDEREADONLY, strFilter);	
	dlgSave.m_ofn.lpstrTitle = _T("Select filename for your notebook");

	//if the user has not selected a plain file, we are done
	if( IDOK != dlgSave.DoModal())
	{
		//clear the member holding the path of the file to be encrypted
		return FALSE;
	}
	else
	{
		*pstrURL = dlgSave.GetPathName();

		//make sure we have the correct file extension
		if(pstrURL->Right( NOTEBOOK_EXT.GetLength() ) != NOTEBOOK_EXT)
		{
			*pstrURL += NOTEBOOK_EXT;
		}
		return TRUE;
	}
}

void CCore::ClearString( CString* pstrData )
{
	ASSERT(pstrData != NULL);

	UINT8* buf;
	int len = pstrData->GetLength();

	buf = (UINT8*)pstrData->GetBuffer();

	SecureZeroMemory(buf, (len*sizeof(wchar_t)));
}



BOOL CCore::HandleCommandLine()
{
	LPWSTR *szArglist;
	int nArgs = 0;
	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	CString strPath;

	//do we have input from the command line?
	if(nArgs == 2)
	{
		//get the input as potential path to a notebook
		strPath = szArglist[1];

		//parse the notebook
		if( ::PathFileExistsW(strPath) )
		{
			AskAndLoadNotebook(strPath);
		}
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CCore::ExportNotebook( )
{
	int i,n;
	CPasswordNotesArray* pNotes;
	CFile fileOut;
	CString strLine, strUrl, strFilterOut;
	CFileException FE;
	BOOL fDone = FALSE;

	strFilterOut.Format(_T("%s (*.%s)|*.%s||"), _T("Text file"), _T("txt"), _T("txt"));

	//build output_filename
	CString strFileName = m_notebook.strURL;
	strFileName.Replace( _T(".knb"), _T("") );
	strFileName += _T(".txt");

	//create file dialog
	CFileDialog save(FALSE, _T("txt"), strFileName, OFN_OVERWRITEPROMPT, strFilterOut);		

	do{

		//did the user select a file path?
		if( IDOK != save.DoModal() )
		{			
			return;
		}

		//yes he did, so get it		
		strUrl = save.GetPathName();
		fDone = TRUE;

	}while(!fDone);


	TRY
	{
		fileOut.Open( strUrl, CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive, &FE );

	}
	CATCH(CFileException, e){

		//something went wrong when reading from the file 
		AfxMessageBox( _T("Could not create the output file.") );
		return;
	}
	END_CATCH

	strLine = _T("type,domain,user,password");
	fileOut.Write( strLine, (sizeof(wchar_t))*strLine.GetLength() );
	fileOut.Write( _T("\r\n"), (sizeof(wchar_t))*2 );

	pNotes = GetPasswordArray(PSWD_ACCOUNT);
	ASSERT(pNotes);
	n = pNotes->GetSize();

	for(i=0; i<n; i++)
	{
		strLine = _T("Account,\"") + pNotes->GetAt(i).strDomain + _T("\",\"") + pNotes->GetAt(i).strInfo + _T("\",\"") + pNotes->GetAt(i).strPassword + _T("\"");
		fileOut.Write( strLine, (sizeof(wchar_t))*strLine.GetLength() );
		fileOut.Write( _T("\r\n"), (sizeof(wchar_t))*2 );
	}

	pNotes = GetPasswordArray(PSWD_CONTACT);
	ASSERT(pNotes);
	n = pNotes->GetSize();

	for(i=0; i<n; i++)
	{
		strLine = _T("Contact,,\"") + pNotes->GetAt(i).strDomain + _T("\",\"") + pNotes->GetAt(i).strPassword +  _T("\"");
		fileOut.Write( strLine, (sizeof(wchar_t))*strLine.GetLength() );
		fileOut.Write( _T("\r\n"), (sizeof(wchar_t))*2 );
	}

	pNotes = GetPasswordArray(PSWD_FILE);
	ASSERT(pNotes);
	n = pNotes->GetSize();

	for(i=0; i<n; i++)
	{
		strLine = _T("File,\"") + pNotes->GetAt(i).strDomain + _T("\",") + _T(",\"") + pNotes->GetAt(i).strPassword + + _T("\"");
		fileOut.Write( strLine, (sizeof(wchar_t))*strLine.GetLength() );
		fileOut.Write( _T("\r\n"), (sizeof(wchar_t))*2 );
	}

	pNotes = GetPasswordArray(PSWD_MESSAGE);
	ASSERT(pNotes);
	n = pNotes->GetSize();

	for(i=0; i<n; i++)
	{
		strLine = _T("Message,\"") + pNotes->GetAt(i).strDomain + _T("\",") + _T(",\"") + pNotes->GetAt(i).strPassword + + _T("\"");
		fileOut.Write( strLine, (sizeof(wchar_t))*strLine.GetLength() );
		fileOut.Write( _T("\r\n"), (sizeof(wchar_t))*2 );
	}

	fileOut.Close();
}


UINT32 CCore::_get_version( UINT32 dwFileType, UINT8 version[SIZE_VERSION_IN_BYTES] )
{
	if( dwFileType > NUM_ENCRYPTED_DATA_TYPES )
	{
		return ERROR_UNKNOWN_FILETYPE;
	}

	switch (dwFileType)
	{
		case DATATYPE_NOTEBOOK:
			version[0] = KNB_CURRENT_ENCODING;
			version[1] = KNB_CURRENT_HASH_VERS;
			version[2] = KNB_CURRENT_VERS_MAJOR;
			version[3] = KNB_CURRENT_VERS_MINOR;
			break;
		case DATATYPE_ENCRYPTEDFILE:
			version[0] = KEF_CURRENT_ENCODING;
			version[1] = KEF_CURRENT_HASH_VERS;
			version[2] = KEF_CURRENT_VERS_MAJOR;
			version[3] = KEF_CURRENT_VERS_MINOR;
			break;
		case DATATYPE_ENCRYPTEDMESSAGE:
			version[0] = KEM_CURRENT_ENCODING;
			version[1] = KEM_CURRENT_HASH_VERS;
			version[2] = KEM_CURRENT_VERS_MAJOR;
			version[3] = KEM_CURRENT_VERS_MINOR;
			break;
	}

	return ERROR_SUCCESS;
}

UINT32 CCore::_parse_version( UINT32 type, UINT8 version[SIZE_VERSION_IN_BYTES] )
{
	UINT8 target_version[SIZE_VERSION_IN_BYTES];

	_get_version( type, target_version );

	if( version[0] > target_version[0] )
	{
		return ERROR_UNSUPPORTED_ENCODING;
	}

	if( version[1] > target_version[1] )
	{
		return ERROR_UNSUPPORTED_PSWD_HASH;
	}

	if( version[2] > target_version[2] )
	{
		return ERROR_UNSUPPORTED_VERSION;
	}

	return ERROR_NONE;
}






UINT32 CCore::EncryptAndEncode( CString *pstrInput, CString *pstrPassword, CString &strOutput, BOOL fStore)
{
	ASSERT(pstrPassword != NULL);
	ASSERT(pstrInput != NULL);

	t_thread_result res;
	CString strTmp, strID;
	int i,n,num_blocks;

	strID = _get_random_message_id();
	res = StartEncryptMessage(pstrPassword, *pstrInput, strOutput, strID, fStore); 

	if(res.dwResult != ERROR_NONE)
	{
		return res.dwResult;
	}

	strTmp = res.strResult;

	n = strTmp.GetLength();
	num_blocks = n/64;

	strOutput = MSG_HEADER;
	strOutput += MSG_ID_HEADER;
	strOutput += strID;
	strOutput += _T("\r\n\r\n");
	for(i=0; i<num_blocks; i++)
	{
		strOutput += strTmp.Mid(i*64, 64);
		strOutput += _T("\r\n");
	}
	strOutput += strTmp.Right(n%64);
	strOutput += MSG_FOOTER;

	return res.dwResult;
}

UINT32 CCore::DecodeAndDecrypt( CString *pstrInput, CString *pstrPassword, CString strID,  CString &strOutput, BOOL fStorePassword )
{
	ASSERT(pstrPassword != NULL);
	ASSERT(pstrInput != NULL);
	int len, index;

	len = pstrInput->GetLength();
	index = pstrInput->Find( _T("KRYP>") );
	pstrInput->Right(len - index);

	t_thread_result res = StartDecryptMessage(pstrPassword,  pstrInput->Right(len - index), strID, fStorePassword);

	if(ERROR_NONE != res.dwResult)
	{
		ReportError(res.dwResult);
		return res.dwResult;
	}

	strOutput = res.strResult;

	return res.dwResult;
}




BOOL CCore::FindPasswordInNotebook( CString &strPassword, UINT8 iv[SIZE_IV_IN_BYTES] )
{
	UINT32 type, i, n;
	CPasswordNotesArray* pNotes;

	for(type=0; type<NUM_PASSWORD_TYPES; type++)
	{
		pNotes = GetPasswordArray(type);
		n = pNotes->GetSize();

		for(i=0; i<n; i++)
		{
			UINT8 tmp1[16];
			UINT8 tmp2[16];
			memcpy(tmp1,  pNotes->GetAt(i).iv ,16);
			memcpy(tmp2,  iv ,16);

			if(!memcmp(iv, pNotes->GetAt(i).iv, SIZE_IV_IN_BYTES))
			{
				strPassword = pNotes->GetAt(i).strPassword;
				return TRUE;
			}
		}
	}

	return FALSE;
}




t_thread_result CCore::Execute( crypt_context *ctx, UINT32 task )
{
	t_thread_result res;
	CWorkerThreadKrypcak thread;
	CProgressDlg progress;
	UINT32 dwRes = ERROR_GENERIC;

	progress.SetHeader(task);
	
	thread.attach(this);
	thread.setContext(ctx);
	thread.setTask(task);
	thread.attach(&progress);

	//start thread
	thread.start();	

	dwRes = progress.DoModal();
	
	//start dlg
	if(IDOK == dwRes )
	{
		res.dwResult = progress.GetResult();
		res.strResult = progress.GetStringResult();
	}

	return res;
}



UINT32 CCore::StartEncryptFile( CString* pstrPassword, CString strURL_In, CString strURL_Out, BOOL fStorePassword )
{
	crypt_context ctx;
	ctx.pstrPassword = pstrPassword;
	ctx.pstrIn = &strURL_In;
	ctx.pstrOut = &strURL_Out;
	ctx.fStorePassword = fStorePassword;

	t_thread_result res = Execute(&ctx, taskEncryptFile); 
	return res.dwResult;
}

BOOL CCore::EncryptFile( crypt_context* pContext, CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	nRes = _encrypt_file( pContext );

	pDlg->SetResult(nRes);

	if(ERROR_NONE == nRes)
	{
		if( pContext->fStorePassword && IsNotebookLoaded() )
		{
			_complete_and_add_file_password( pContext->pstrPassword, *pContext->pstrOut, pContext->iv );
		}
	}

	return TRUE;
}



UINT32 CCore::StartDecryptFile( CString* pstrPassword, CString strURL_In, CString strURL_Out, BOOL fStorePassword )
{
	crypt_context ctx;
	ctx.pstrPassword = pstrPassword;
	ctx.pstrIn = &strURL_In;
	ctx.fStorePassword = fStorePassword;
	ctx.pstrOut = &strURL_Out;

	t_thread_result res = Execute(&ctx, taskDecryptFile); 
	return res.dwResult;
}

BOOL CCore::DecryptFile( crypt_context* pContext, CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	nRes = _decrypt_file( pContext );

	pDlg->SetResult(nRes);

	if(ERROR_NONE == nRes)
	{
		if( pContext->fStorePassword && IsNotebookLoaded() )
		{
			_complete_and_add_file_password( pContext->pstrPassword, *pContext->pstrIn, pContext->iv );
		}
	}

	return TRUE;
}

BOOL CCore::LoadNotebook( crypt_context* pContext, CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	nRes = _parse_notebook_from_file( *pContext->pstrIn, pContext->pstrPassword );

	pDlg->SetResult(nRes);

	return TRUE;
}

UINT32 CCore::StartLoadNotebook( CString strURL, CString* pstrPassword )
{
	crypt_context ctx;
	ctx.pstrPassword = pstrPassword;
	ctx.pstrIn = &strURL;

	t_thread_result res = Execute(&ctx, taskLoadNotebook); 
	return res.dwResult;
}

BOOL CCore::SaveNotebook( CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	nRes = _write_notebook_to_file();

	pDlg->SetResult(nRes);

	return TRUE;

}

UINT32 CCore::StartSaveNotebook()
{
	crypt_context ctx;
	ctx.pstrPassword = NULL;
	ctx.pstrIn = NULL;
	ctx.fStorePassword = FALSE;

	t_thread_result res = Execute(&ctx, taskSaveNotebook); 
	return res.dwResult;

}

t_thread_result CCore::StartEncryptMessage( CString* pstrPassword, CString strIn, CString strOut, CString strID, BOOL fStorePassword )
{
	crypt_context ctx;
	ctx.pstrPassword = pstrPassword;
	ctx.pstrIn = &strIn;
	ctx.pstrOut = &strOut;
	ctx.fStorePassword = fStorePassword;
	ctx.pstrInfo = &strID;

	return Execute(&ctx, taskEncryptMessage);
}

BOOL CCore::EncryptMessage( crypt_context* pContext, CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	nRes = _encrypt_and_encode_string( pContext );

	pDlg->SetResult(nRes);

	if(ERROR_NONE == nRes)
	{
		pDlg->SetResult(*pContext->pstrOut);

		if( pContext->fStorePassword && IsNotebookLoaded() )
		{
				note_password pswd;
				pswd.dwType = PSWD_MESSAGE;
				pswd.strPassword = *pContext->pstrPassword;
				pswd.strDomain = *pContext->pstrInfo;
				pswd.strDate = GetCurrentDateAsString();
				memcpy(pswd.iv, pContext->iv, SIZE_IV_IN_BYTES);

				AddPassword(&pswd);
		}
	}

	return TRUE;
}

t_thread_result CCore::StartDecryptMessage( CString* pstrPassword, CString strIn, CString strID, BOOL fStorePassword )
{
	crypt_context ctx;
	CString strTmp;
	ctx.pstrPassword = pstrPassword;
	ctx.pstrIn = &strIn;
	ctx.pstrOut = &strTmp;
	ctx.fStorePassword = fStorePassword;
	ctx.pstrInfo = &strID;

	return Execute(&ctx, taskDecryptMessage);
}

BOOL CCore::DecryptMessage( crypt_context* pContext, CProgressDlg *pDlg )
{
	UINT32 nRes=0;
	m_hWndStatus = pDlg->m_hWnd;

	
	nRes = _decode_and_decrypt( pContext );

	pDlg->SetResult(nRes);

	if(ERROR_NONE == nRes)
	{
		pDlg->SetResult(*pContext->pstrOut);

		if( pContext->fStorePassword && IsNotebookLoaded() )
		{
			note_password pswd;
			pswd.dwType = PSWD_MESSAGE;
			pswd.strPassword = *pContext->pstrPassword;
			pswd.strDomain = *pContext->pstrInfo;
			pswd.strDate = GetCurrentDateAsString();
			memcpy(pswd.iv, pContext->iv, SIZE_IV_IN_BYTES);

			AddPassword(&pswd);
		}
	}
	
	return TRUE;
}



BOOL CCore::TestCrypto()
{
	Keccak_DuplexInstance test;	
	UINT8 output[KECCAK_IO_SIZE_BYTES];

	// initialize the keccak duplex object as will be used by Krypcak
	// Keccak_DuplexInitialize(&test, KECCAK_RATE_BITS, KECCAK_CAPACITY_BITS);
	Keccak_DuplexInitialize(&test, KECCAK_RATE_BITS, KECCAK_CAPACITY_BITS);

	/*
		we will absorb data of different bitsizes as described in the 
		KeccakCodePackage-master 
	*/
	Keccak_Duplexing(&test, NULL        , NUM_TEST_BYTES_0, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_0);
	Keccak_Duplexing(&test, TEST_INPUT_1, NUM_TEST_BYTES_1, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_1);
	Keccak_Duplexing(&test, TEST_INPUT_2, NUM_TEST_BYTES_2, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_2);
	Keccak_Duplexing(&test, TEST_INPUT_3, NUM_TEST_BYTES_3, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_3);
	Keccak_Duplexing(&test, TEST_INPUT_4, NUM_TEST_BYTES_4, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_4);
	Keccak_Duplexing(&test, TEST_INPUT_5, NUM_TEST_BYTES_5, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_5);	
	Keccak_Duplexing(&test, TEST_INPUT_6, NUM_TEST_BYTES_6, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_6);
	Keccak_Duplexing(&test, TEST_INPUT_7, NUM_TEST_BYTES_7, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_7);
	Keccak_Duplexing(&test, TEST_INPUT_8, NUM_TEST_BYTES_8, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_8);

	/*
		now absorb a 'full' block and compare with the expected value from the
		testvector from the KeccakCodePackage-master
	*/
	Keccak_Duplexing(&test, TEST_INPUT_9, NUM_TEST_BYTES_9, output, KECCAK_IO_SIZE_BYTES, LAST_BYTE_9);

	if( memcmp(output, TEST_OUTPUT, 128) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*
	report the error to the user
*/
void CCore::ReportError( UINT32 dwError )
{
	switch (dwError)
	{
	case ERROR_INCORRECT_PSWD:
		AfxMessageBox(IDS_ERR_INC_PSWD);
		break;
	case ERROR_INVALID_FILE:
		AfxMessageBox(IDS_INVALID_FILE);
		break;
	case ERROR_UNSUPPORTED_VERSION:
		AfxMessageBox(IDS_ERROR_UNSUPPORTED_VERSION);
		break;
	case ERROR_UNSUPPORTED_PSWD_HASH:
		AfxMessageBox(IDS_ERROR_UNSUPPORTED_PSWD_HASH);
		break;
	case ERROR_UNSUPPORTED_ENCODING:
		AfxMessageBox(IDS_ERROR_UNSUPPORTED_ENCODING);
		break;
	case ERROR_CORRUPT_FILE:
		AfxMessageBox(IDS_ERROR_CORRUPT_FILE);
		break;
	case ERROR_CORRUPT_MESSAGE:
		AfxMessageBox(IDS_ERROR_CORRUPT_MESSAGE);
		break;
	case ERROR_READ_FROM_FILE:
		AfxMessageBox(IDS_ERROR_READ_FILE);
		break;
	case ERROR_WRITE_TO_FILE:
		AfxMessageBox(IDS_ERROR_WRITE_FILE);
		break;
	case ERROR_PLAIN_MESSAGE_TOO_LARGE:
		AfxMessageBox(IDS_MESSAGE_TOO_LONG);
		break;
	case ERROR_ENC_MESSAGE_TOO_LARGE:
		AfxMessageBox(IDS_MESSAGE_TOO_LONG);
		break;
	case ERROR_PSWD_TOO_LARGE:
		AfxMessageBox( IDS_MAX_PSWD_LENGTH );
		break;
	case ERROR_NON_MATCHING_PSWDS:
		AfxMessageBox(IDS_NO_MATCH);
		break;
	case ERROR_CRYPTO:
		AfxMessageBox(IDS_ERROR_CRYPTO);
		break;
	case ERROR_NOTEBOOK_FULL:
		AfxMessageBox(IDS_ERROR_NOTEBOOK_FULL);
	case ERROR_NO_PLAIN_MESSAGE:
		AfxMessageBox(IDS_ERROR_NO_PLAIN_MESSAGE);
	case ERROR_NO_ENC_MESSAGE:
		AfxMessageBox(IDS_ERROR_NO_ENC_MESSAGE);
		break;
	default:
		AfxMessageBox( _T("The operation could not be completed.") );
		break;
	}
}

/*
	update the progress bar
*/
void CCore::_report_progress( WPARAM wParam, LPARAM lParam )
{
	if( m_hWndStatus != NULL) 
	{
		PostMessage( m_hWndStatus, WM_KRYPCAK_PROGRESS, wParam, lParam);
	}
}

