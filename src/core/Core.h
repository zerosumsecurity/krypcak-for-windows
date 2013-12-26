#pragma once
#include "../afx/stdafx.h"
#include "../resources/woorden.h"
#include "../dialogs/ProgressDlg.h"
#include "../prng/KrypcakPRNG.h"
#include "../dialogs/WelcomeDlg.h"
#include "../dialogs/SetPasswordDlg.h"
#include "../keccak/KeccakDuplexTestVector.h"


extern "C"
{
	#include "../keccak/KeccakDuplex.h"
}


#define KNB_CURRENT_ENCODING	0x00
#define KNB_CURRENT_HASH_VERS	0x00
#define KNB_CURRENT_VERS_MAJOR	0x01
#define KNB_CURRENT_VERS_MINOR	0x00

#define KEF_CURRENT_ENCODING	0x00
#define KEF_CURRENT_HASH_VERS	0x00
#define KEF_CURRENT_VERS_MAJOR	0x01
#define KEF_CURRENT_VERS_MINOR	0x00

#define KEM_CURRENT_ENCODING	0x00
#define KEM_CURRENT_HASH_VERS	0x00
#define KEM_CURRENT_VERS_MAJOR	0x01
#define KEM_CURRENT_VERS_MINOR	0x00

#define NUM_ENCRYPTED_DATA_TYPES	3
const enum{
	DATATYPE_NOTEBOOK,
	DATATYPE_ENCRYPTEDFILE,
	DATATYPE_ENCRYPTEDMESSAGE
};

//time before clipboard erased
#define NUM_SECS_PSWD_ON_CLIPBOARD	10

//default password settings
#define DEFAULT_PASSWORD_LENGTH			16
#define DEFAULT_PASSWORD_ALPHABET_MASK	0x7

//keccak defines
#define KECCAK_RATE_BITS			1027
#define KECCAK_CAPACITY_BITS		573
#define KECCAK_IO_SIZE_BYTES		128

//password hashing defines
#define NUM_ITERATIONS_PSWD_INIT_SQUEEZE	(1<<19) //usage = 2^{19}*2^{7} = 2^{26} bytes = is 64 MB RAM
#define NUM_ITERATIONS_PSWD_INIT_ABSORB		NUM_ITERATIONS_PSWD_INIT_SQUEEZE
#define SIZE_LARGE_PSWD_HASH_BUFFER		(KECCAK_IO_SIZE_BYTES*NUM_ITERATIONS_PSWD_INIT_SQUEEZE)

#define NUM_PSWD_PROGRESS_STEPS			10
#define INCREMENT_PSWD_PROGRESS_STEP	(100/NUM_PSWD_PROGRESS_STEPS)
#define PSWD_PROGRESS_STEPS_TRIGGER		((NUM_ITERATIONS_PSWD_INIT_ABSORB+NUM_ITERATIONS_PSWD_INIT_SQUEEZE)/NUM_PSWD_PROGRESS_STEPS)
#define NUM_EXTERNAL_BYTES				128

#define MAX_PSWD_SIZE				255
#define MAX_PHRASE_SIZE				16

//settings for message id generation
#define MSG_ID_SIZE					5
#define MSG_ID_ALPHABET_MSK			0x01 //0x01 = just numbers

#define ZERSOSUM_DARK				RGB(0x3b,0x3d,0x40)

//pre-defined sizes
#define SIZE_MARK_IN_BYTES			4
#define SIZE_VERSION_IN_BYTES		4
#define SIZE_IV_IN_BYTES			16
#define PSWD_CHECK_SIZE_BYTES		4
#define SIZE_MAC_IN_BYTES			16
#define HEADER_SIZE_BYTES			(SIZE_MARK_IN_BYTES+SIZE_VERSION_IN_BYTES+SIZE_IV_IN_BYTES+PSWD_CHECK_SIZE_BYTES)
#define SIZE_OVERHEAD_IN_BYTES		(HEADER_SIZE_BYTES+SIZE_MAC_IN_BYTES) 

//error codes
#define ERROR_NONE						0x00
#define ERROR_WRITE_TO_FILE				0xe00
#define ERROR_READ_FROM_FILE			0xe01
#define ERROR_INVALID_FILE				0xe02
#define ERROR_CORRUPT_FILE				0xe03
#define ERROR_NOTEBOOK_TOO_LARGE		0xe04
#define ERROR_ENTRY_TOO_LARGE			0xe05
#define ERROR_CANT_FIND_FILE			0xe06
#define ERROR_MULT_FILES_FOUND			0xe07
#define ERROR_NO_NOTEBOOK_LOADED		0xe08
#define ERROR_INCORRECT_PSWD			0xe09
#define ERROR_NOTEBOOK_UNCHANGED		0xe0a
#define ERROR_UNKNOWN_FILETYPE			0xe0b
#define ERROR_FILEFORMAT_NOT_SUP		0xe0c
#define ERROR_UNSUPPORTED_VERSION		0xe0d
#define ERROR_UNSUPPORTED_PSWD_HASH		0xe0e
#define ERROR_UNSUPPORTED_DOMAIN		0xe0f
#define ERROR_GENERIC					0xe10
#define ERROR_UNSUPPORTED_ENCODING		0xe11
#define ERROR_CORRUPT_MESSAGE			0xe12
#define ERROR_ENC_MESSAGE_TOO_LARGE		0xe13
#define ERROR_PLAIN_MESSAGE_TOO_LARGE	0xe14
#define ERROR_PSWD_TOO_LARGE			0xe15	
#define ERROR_NON_MATCHING_PSWDS		0xe16
#define ERROR_DECODE_BASE64				0xe17
#define ERROR_NO_MSG_ID					0xe18
#define ERROR_CRYPTO					0xe19
#define ERROR_PSWD_NOT_FOUND			0xe1a
#define ERROR_NOTEBOOK_FULL				0xe1b
#define ERROR_NO_PLAIN_MESSAGE			0xe1c
#define ERROR_NO_ENC_MESSAGE			0xe1d

#define MAX_LENGTH_DLG_HEADER		40

#define MAX_NUM_ENTRIES_NOTEBOOK	8192
#define MAX_SIZE_NOTE_BYTES			2048
#define MAX_SIZE_NOTEBOOK_BYTES		(MAX_NUM_ENTRIES_NOTEBOOK*MAX_SIZE_NOTE_BYTES)

#define MAX_NUM_CHARS_PLAIN_MESSAGE		65536
#define MAX_NUM_CHARS_ENCRYPTED_MESSAGE	(3*MAX_NUM_CHARS_PLAIN_MESSAGE)

//#define HIDDEN_PSWD_CHAR		_T('\u25CF')
#define HIDDEN_PSWD_CHAR		0x25cf


#define NUM_PASSWORD_TYPES			4
const enum PASSWORD_TYPE{
	PSWD_FILE,
	PSWD_MESSAGE,
	PSWD_CONTACT,
	PSWD_ACCOUNT

};

const enum PASSWORD_USE{
	PSWD_VIEW,
	PSWD_SELECT
};

const enum SET_PSWD_MODE{
	PSWD_ENC,
	PSWD_DEC
};

typedef struct t_note_password{
	UINT32 dwSize;					// 4 bytes
	UINT32 dwType;					// 4 bytes
	UINT32 dwData;					// 4 bytes 
	UINT32 dwStrDateByteLength;     // 4 bytes
	CString	strDate;
	UINT32 dwStrDomainByteLength;	// 4 bytes
	CString strDomain;
	UINT32 dwStrInfoByteLength;		// 4 bytes
	CString strInfo;
	UINT32 dwStrPasswordByteLength;	// 4 bytes
	CString strPassword;
	UINT8 iv[SIZE_IV_IN_BYTES];
} note_password;					// overhead is 28 bytes
// expected size is in the order of 28 + 20 + + 30 + 30 + 30 = 138 bytes  

typedef CArray<note_password, note_password> CPasswordNotesArray;

typedef struct t_notebook{
	UINT32 total_byte_size;
	UINT32 num_entries;
	CPasswordNotesArray passwords_accounts;
	CPasswordNotesArray passwords_contacts;
	CPasswordNotesArray passwords_files;
	CPasswordNotesArray passwords_messages;
	CPasswordNotesArray passwords_notes;
	CString strURL;
	CString strPassword;
	BOOL changed_after_loading;
} notebook;

typedef struct t_thread_result{
	CString strResult;
	UINT32 dwResult;
} thread_result;

typedef struct t_format_version{
	UINT8 minor;
	UINT8 major;
	UINT8 hash_type;
	UINT8 domain;
} format_version;

//4-byte file markers
static const UINT8 MARK_ENCFILE[4] =		{0x5A,0x53,0x45,0x46}; // "ZSEF"
static const UINT8 MARK_ENCNOTEBOOK[4] =	{0x5A,0x53,0x4E,0x42}; // "ZSNB"
static const UINT8 MARK_ENCMESSAGE[4] =		{0x5A,0x53,0x45,0x4d}; // "ZSEM"

static const CString ENC_FILE_EXT = _T(".kef");
static const CString NOTEBOOK_EXT = _T(".knb");

//strings 
static const CString WINDOW_TEXT = _T(" Krypcak Password Tool");

static const CString BASE64_ALPHABET = _T("ABCDEPGHIJWLMYOFQlS>UVKXNZabcdefghijkRmnopqrstuvwxyz0123456789<T");

static const CString MSG_HEADER = _T("--BEGIN KRYPCAK ENCRYPTED MESSAGE--\r\n");
static const CString MSG_FOOTER = _T("\r\n\r\n---END KRYPCAK ENCRYPTED MESSAGE---");
static const CString MSG_ID_HEADER = _T("Message id: ");


const enum LANGUAGES{
	DUTCH,
	ENGLISH,
	RFC2289
};

const enum MODES{
	taskLoadNotebook,
	taskSaveNotebook,
	taskEncryptFile,
	taskDecryptFile,
	taskEncryptMessage,
	taskDecryptMessage,
};

typedef struct crypt_context{
	Keccak_DuplexInstance *pSponge;
	CString *pstrPassword;
	UINT8 iv[SIZE_IV_IN_BYTES];
	UINT32 mode;
	CString *pstrIn;
	CString *pstrOut;
	BOOL fStorePassword;
	CString *pstrInfo;
}t_crypt_ctx;


class CCore
{
public:
	CCore(void);
	virtual ~CCore(void);

	BOOL TestCrypto();

	void CloseAndCleanUp();
	
	/*
		public operations on t_note_password objects
	*/
	void CleanPassword(t_note_password* pPswd);
	void _set_data_from_password(note_password* pPassword);
	void _set_sizes(note_password* pPassword);
	UINT32 _compute_checksum(t_note_password *pPswd);
	BOOL _check_password(t_note_password* pPswd);
	void _complete_file_password(t_note_password* pPswd, CString* pstrPassword, CString strURL, UINT8 iv[SIZE_IV_IN_BYTES]);

	/*
		public operations on m_notebook
	*/
	
	BOOL AskAndLoadNotebook(CString strStartURL);
	BOOL GetLastUsedNotebookURL(CString &strURL);
	CString GetLoadedNotebookURL();
	BOOL SelectNotebookURL(CString &strURL);
	BOOL SetNewNotebookURL(CString strURL);
	BOOL SetNewNotebookPassword(CString* pstrPassword);
	void CleanNotebook();
	void SetNotebookLoadedFlag(BOOL flag);
	BOOL AddPassword(note_password *pPassword);
	BOOL _complete_and_add_file_password(CString* pstrPassword, CString strURL, UINT8 iv[SIZE_IV_IN_BYTES]);
	BOOL RemovePassword(note_password* pPswd);
	UINT32 _compute_notebook_size_in_bytes();
	UINT32 _compute_num_entries_notebook();
	BOOL SelectNewNotebookURL(CString *pstrURL);
	BOOL IsNotebookLoaded(){return m_fNotebookLoaded;}
	UINT32 _write_notebook_to_file();
	UINT32 _parse_notebook_from_file( CString strURL, CString* pstrPassword);
	int _encrypt_buffer_to_file(CString* pstrPassword, UINT8* buf, UINT32 num_bytes, CFile* pfOUT );
	int _parse_notebook_from_buffer(UINT8* buf, UINT32 size);
	int _decrypt_file_into_buffer(CString* pstrPassword, UINT8* buf, UINT32 &num_bytes, CFile* pfIN );
	void ChangePasswordNotebook();
	CPasswordNotesArray* GetPasswordArray(UINT32 dwType);
	
	void ExportNotebook();
	BOOL FindPasswordInNotebook(CString &strPassword, UINT8 iv[SIZE_IV_IN_BYTES]);

	/*
		general helper functions
	*/
	CString GetTemporaryFileName(CString strTarget);
	CString GetCurrentDateAsString();
	void CopyStringToClipBoard(CString* pstrData, BOOL fWipe = TRUE);
	static void DelayedEmptyClipBoard(void* );
	static void ClearString(CString* pstrData);
	UINT32 _get_version(UINT32 dwFileType, UINT8 version[SIZE_VERSION_IN_BYTES]);
	UINT32 _parse_version(UINT32 type, UINT8 version[SIZE_VERSION_IN_BYTES]);
	void ReportError(UINT32 dwError);
	/*
		command line
	*/
	BOOL HandleCommandLine();

	/*
		messages
	*/


	UINT32 _base64_encode(UINT8 *buf, UINT32 dwInputength, CString &strOutput);
	UINT32 _base64_decode(CString *pstrInput, UINT8* buf, UINT32 *dwOutputLength);

	/*
		threads
	*/

	UINT32 SetUpCrypto(crypt_context *ctx);
	
	t_thread_result Execute(crypt_context *ctx, UINT32 task);

	t_thread_result StartEncryptMessage(CString* pstrPassword, CString strIn, CString strOut, CString strID, BOOL fStorePassword );
	BOOL EncryptMessage(crypt_context* pContext, CProgressDlg *pDlg);

	t_thread_result StartDecryptMessage(CString* pstrPassword, CString strIn, CString strID, BOOL fStorePassword );
	BOOL DecryptMessage(crypt_context* pContext, CProgressDlg *pDlg);

	
	UINT32 StartEncryptFile(CString* pstrPassword, CString strURL_In, CString strURL_Out, BOOL fStorePassword );
	BOOL EncryptFile(crypt_context* pContext, CProgressDlg *pDlg);

	UINT32 StartDecryptFile(CString* pstrPassword, CString strURL_In, CString strURL_Out, BOOL fStorePassword);
	BOOL DecryptFile(crypt_context* pContext, CProgressDlg *pDlg);

	UINT32 StartLoadNotebook( CString strURL, CString* pstrPassword );
	BOOL LoadNotebook(crypt_context* pContext, CProgressDlg *pDlg);

	UINT32 StartSaveNotebook();
	BOOL SaveNotebook( CProgressDlg *pDlg);



	void _report_progress( WPARAM wParam, LPARAM lParam );

	/*
		interface to the PRNG
	*/
	void UpdatePRNG();
	int GetRandom( UINT8* output, int num_bytes );
	BOOL _sufficent_entropy_gathered();

	void MixInMousePoint(CPoint* pPoint);

	/*
		file encryption and decryption
	*/
	int _encrypt_file( crypt_context *ctx );
	int _decrypt_file( crypt_context *ctx );
	int _encrypt_file( CString* pstrPassword, CFile* pfIN, CFile* pfOUT, UINT8 iv[SIZE_IV_IN_BYTES]);
	int _decrypt_file( CString* pstrPassword, CFile* pfIN, CFile* pfOUT, UINT8 iv[SIZE_IV_IN_BYTES]);
	BOOL _get_iv_from_encrypted_file(CFile* fIN, UINT8 iv[SIZE_IV_IN_BYTES]);
	UINT32 PreParseEncryptedMessage(CString *pstrMessage, CString &strID, UINT8 iv[SIZE_IV_IN_BYTES]);

	UINT32 FindPasswordForFile(CString* pstrPassword, CString strURL);
	void StripDecryptFileName(CString &strURL);

	UINT32 EncryptAndEncode(CString *pstrInput, CString *pstrPassword, CString &strOutput, BOOL fStore);
	UINT32 _encrypt_and_encode_string( crypt_context *ctx);

	UINT32 DecodeAndDecrypt(CString *pstrInput, CString *pstrPassword, CString strID, CString &strOutput, BOOL fStorePassword);
	UINT32 _decode_and_decrypt( crypt_context *ctx );

	CString _get_random_message_id();
	UINT32 _find_message_id(CString &strID, CString* pstrBase64EncodedMessage);
	/*
		passwords and passphrases
	*/
	void GeneratePassword(CString &strPassword, int password_length, UINT8 bAlphabetMask);
	void GeneratePassphrase(CString &strPassword, UINT32 num_words, UINT8 language_index);

	/*
		interface with the registry
	*/
	void SaveLastUsedNotebookURLToRegistry(CString strURL);
	BOOL RetrieveLastUsedNotebookURLToRegistry(CString &strURL);

	/*
		password hashing
	*/
	int _absorb_password_and_iv(Keccak_DuplexInstance* pSponge, CString* pstrPassword, UINT8 salt[SIZE_IV_IN_BYTES]);
	int _set_up_crypto(Keccak_DuplexInstance* pSponge, CString* pstrPassword, UINT8 salt[SIZE_IV_IN_BYTES], UINT8 check[PSWD_CHECK_SIZE_BYTES]);
	
	/*
		private members
	*/
private:
	HWND			m_hWndStatus;

	CKrypcakPRNG		m_prng; //the pseudo random number generator
	t_notebook		m_notebook; //notebook holding the passwords
	BOOL			m_fNotebookLoaded; //flag to indicate a notebook is loaded
};

