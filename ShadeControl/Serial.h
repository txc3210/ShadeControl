#ifndef _DLL_SAMPLE_H
#define _DLL_SAMPLE_H

// ͨ�����������ǵ��뻹�ǵ���
#ifdef SERIAL_EXPORTS
#define DLL_Serial_API __declspec(dllexport)
#else
#define DLL_Serial_API __declspec(dllimport)
#endif

// ����/�����������


#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13


class DLL_Serial_API CSerial
{
	
public:
	int SendDelayData( /*const*/unsigned char *buffer, int size );
	BOOL Clear();
	CSerial();
	~CSerial();
	
	BOOL Open( int nPort = 2, int nBaud = 9600 );
	BOOL Close( void );
	
	int ReadData( void *, int );
	int SendData( unsigned char *, int );
	
	int ReadDataWaiting( void );
	
	//BOOL IsOpened( void ){ return( m_bOpened ); }
	BOOL IsOpened( void );
	
	BOOL WriteCommByte(unsigned char *lpCommand, DWORD dwBytesToWrite);
	
protected:
	BOOL WriteCommByte( unsigned char );
	HANDLE m_hIDComDev;
	OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
	BOOL m_bOpened;
	
};

#endif
