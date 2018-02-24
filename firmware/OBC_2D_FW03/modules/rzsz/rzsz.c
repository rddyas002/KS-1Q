#include <stdio.h>
#include "os.h"
#include "rzsz.h"
#include "uart.h"

#define TRUE 1
#define FALSE 0
#define OK 0
#define ERROR (-1)
#define TIMEOUT (-2)
#define RCDO (-3)
#define GCOUNT (-4)
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025

#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)

#define RETRYMAX 5
#define WCEOT (-10)
#define WANTCRC 0103	/* send C not NAK to get crc not checksum */
#define BUFF_SIZE 512
#define CPMEOF 032
#define PATHLEN 257
#define HOWMANY 96
#define DEFBYTL 2000000000L	/* default rx file size */
#define UNIXFILE 0xF000	/* The S_IFMT file mask bit for stat */

/*CRC part*/
#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)
#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF))

static unsigned short crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

static unsigned long cr3tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


static int Zrwindow = 1400; // RX window size(controls garbage count)
static int Zctlesc; // encode control characters
static char Lzconv; //local ZMODEM file conversion request
static char Lzmanag;
/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */
//static char *Zendnames[] = { "ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"};

int errno;

unsigned int Baudrate = Z_BAUD;    //in ZMODEM Protocol
int errors;
int Crc32t; //controls 32 bit CRC being sent CRC32 == 1 CRC32 +RLE ==2
int Crc32r;		/* Indicates/controls 32 bit CRC being received */
			/* 0 == CRC16,  1 == CRC32,  2 == CRC32 + RLE */
int Eofseen;
int Lastrx;
int Rxframeind; //ZBIN ZBIN32 or ZHEX type or frame
int Rxtype; // type of header received
int Rxhlen; //length of header received
int Zctlesc;
int Firstsec;
int Blklen;
int Rxcount; //count of data bytes received
int Filemode; // unix style mode for incoming file
int Thisbinary; //current file is to be received in bin mode
int Usevhdrs;		/* Use variable length headers */
int Rxbinary = FALSE; //receive all files in bin mode
int Rxascii = FALSE; // receive all files in ascii mode 

int Zmodem = 0;
int Nozmodem = 0;	/* If invoked as "rb" */
int Crcflg = 1;
int tryzhdrtype = ZRINIT;
int Lleft = 0;
int Readnum = HOWMANY;
int Rxtimeout = 100;

long rxbytes;
long Filesleft;
long Totalleft;
long Bytesleft;
long Modtime; //unix style mod time for incoming file
long Rxpos;

char zconv; // ZMODEM file conversion request
char zmanag; //ZMODEM file management request
char ztrans; //MODEM file transport request 

UINT btw;
UINT* bw;

char Pathname[PATHLEN];
char buffer[BUFF_SIZE];
char Rxhdr[ZMAXHLEN]; //received header
char Txhdr[ZMAXHLEN]; //transmitted header
char linbuf[HOWMANY];
char Attn[ZATTNLEN+1];
char secbuf[1025];
char endmsg[90] = {0}; //possible message to display on exit
static char badcrc[] = "Bad CRC";

static FIL fout;
FILINFO* f;
//FILE *fout;
//jmp_buf tohere;

/*需要串口通信发送指令*/
void sendline(char c){
	/*串口发送一个字符的函数*/
     uart_putc(SERIAL_PORT, c);
}

/*需要串口通信接收指令*/
int readline(int timeout){
	register int n;
	static char *cdq; //pointer for removing chars from linbuf
	if(--Lleft >= 0){
		return (*cdq++ & 0377);   //和255相与 取低八位
	}
	
	n = timeout/10;
	if(n < 2)
		n = 2;
        /*
	 if(setjmp(tohere)){
		 Lleft = 0;
		 return TIMEOUT;
	 }*/
	 //超过时间n就当为超时
	//signal(SIGALRM, alrm)
	//alarm(n)

	
	errno = 0;
        //vtimer_set(8,n*1000);//开定时器，定时器号为8，定时时长为n秒
        
	TickType_t ticks = xTaskGetTickCount ();
	/*读取信息 用STM32读取函数代替*/
        while( Lleft <1){
		cdq=linbuf;
        Lleft = uart_read(SERIAL_PORT, cdq, Readnum);   //这里改成了console 
        /*
          printf("The reading data is %s \r\n", cdq);*/
        //if(vtimer_ovf(8))//是否时间到
        //{
		vTaskDelay(5);
		TickType_t delta = xTaskGetTickCount() - ticks;
		if(delta > 100)
			return TIMEOUT;
        //}
        }
        
	//delay_nms(n*2000);
	//alarm(0); //取消闹钟
	if(Lleft < 1)
		return TIMEOUT;
	--Lleft;
	return (*cdq++ & 0377); //取cdq的值低八位 cdq地址再自增 
}

/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */
void zmputs(char *s)
{
	register int c;

	while (*s) {
		switch (c = *s++) {
		case '\336':
           vTaskDelay(100);
			//sleep(1); 
			continue;
		case '\335':
			//sendbrk();      //no break signal available 
			continue;
		default:
			sendline(c);
		}
	}
	//flushmo();
}

/* send cancel string to get the other end to shut up */
void canit()
{
	static char canistr[] = {
	 24,24,24,24,24,24,24,24,24,24,8,8,8,8,8,8,8,8,8,8,0
	};

	zmputs(canistr);
	Lleft=0;	/* Do read next time ... */
}

long getfree()
{
	return(2147483647);	/* many free bytes ... */
}

/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
int putsec(char *buf, register int n)
{
	register char *p;
        int ret;
	if (n == 0)
		return OK;
	if (Thisbinary) {
		/*for (p=buf; --n>=0; )
                     f_putc( *p++, &fout);*/
                     ret = f_write (&fout, buf, (UINT)n, bw);
			
	}
	else {
		if (Eofseen)
			return OK;
		for (p=buf; --n>=0; ++p ) {
			if ( *p == '\r' || '\n')
				continue;
			if (*p == CPMEOF) {
				Eofseen = TRUE; 
				return OK;
			}
			f_putc(*p ,&fout);
		}
	}
	return OK;
}



/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
int zdlread()
{
	register int c;

again:
	/* Quick check for non control characters */
	if ((c = readline(Rxtimeout)) & 0140)
		return c;
	switch (c) {
	case ZDLE:
		break;
	case 023:
	case 0223:
	case 021:
	case 0221:
		goto again;
	default:
		if (Zctlesc && !(c & 0140)) {
			goto again;
		}
		return c;
	}
again2:
	if ((c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	switch (c) {
	case CAN:
		return GOTCAN;
	case ZCRCE:
	case ZCRCG:
	case ZCRCQ:
	case ZCRCW:
		return (c | GOTOR);
	case ZRUB0:
		return 0177;
	case ZRUB1:
		return 0377;
	case 023:
	case 0223:
	case 021:
	case 0221:
		goto again2;
	default:
		if (Zctlesc && ! (c & 0140)) {
			goto again2;
		}
		if ((c & 0140) ==  0100)
			return (c ^ 0100);
		break;
	}
	return ERROR;

}

int zrdat32(register char *buf, int length)
{
	register int c;
	register unsigned long crc;
	register char *end;
	register int d;

	crc = 0xFFFFFFFFL;  
	Rxcount = 0;  
	end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				d = c;  
				c &= 0377;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if (crc != 0xDEBB20E3) {
                                        printf("%s\n",badcrc);
					//printf(badcrc);
					return ERROR;
				}
				Rxcount = length - (end - buf);

				return d;
			case GOTCAN:
				printf("Sender Canceled\r\n");
				return ZCAN;
			case TIMEOUT:
				printf("TIMEOUT\r\n");
				return c;
			default:
				//garbitch(); 
				return c;
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	//zperr1("Data subpacket too long");
	return ERROR;
}

/* Receive data subpacket RLE encoded with 32 bit FCS */
int zrdatr32(register char *buf, int length)
{
	register int c;
	register unsigned long crc;
	register char *end;
	register int d;

	crc = 0xFFFFFFFFL;  
	Rxcount = 0;  
	end = buf + length;
	d = 0;	/* Use for RLE decoder state */
	while (buf <= end) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				d = c;  c &= 0377;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = UPDC32(c, crc);
				if (crc != 0xDEBB20E3) {
                                        printf("%s\n", badcrc);
					//zperr1(badcrc);
					return ERROR;
				}
				Rxcount = length - (end - buf);

				return d;
			case GOTCAN:
				//zperr1("Sender Canceled");
				return ZCAN;
			case TIMEOUT:
                                printf("TIMEOUT\n");
				//zperr1("TIMEOUT");
				return c;
			default:
                                printf("Bad data subpacket\n");
				//zperr1("Bad data subpacket");
				return c;
			}
		}
		crc = UPDC32(c, crc);
		switch (d) {
		case 0:
			if (c == ZRESC) {
				d = -1;  continue;
			}
			*buf++ = c;  continue;
		case -1:
			if (c >= 040 && c < 0100) {
				d = c - 035; c = 040;  
				goto spaces;
			}
			if (c == 0100) {
				d = 0;
				*buf++ = ZRESC;  
				continue;
			}
			d = c;  
			continue;
		default:
			d -= 0100;
			if (d < 1)
				goto badpkt;
spaces:
			if ((buf + d) > end)
				goto badpkt;
			while ( --d >= 0)
				*buf++ = c;
			d = 0;  
			continue;
		}
	}
badpkt:
	//zperr1("Data subpacket too long");
	return ERROR;
}


int zrdata(register char *buf, int length){
	register int c;
	register unsigned short crc;
	register char *end;
	register int d;

	switch (Crc32r) {
	case 1:
		return zrdat32(buf, length);
	case 2:
		return zrdatr32(buf, length);
	}

	crc = Rxcount = 0;  
	end = buf + length;
	while (buf <= end) {
		if ((c = zdlread()) & ~0377) {
crcfoo:
			switch (c) {
			case GOTCRCE:
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				crc = updcrc((d=c)&0377, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = updcrc(c, crc);
				if ((c = zdlread()) & ~0377)
					goto crcfoo;
				crc = updcrc(c, crc);
				if (crc & 0xFFFF) {
					//zperr1(badcrc);
					return ERROR;
				}
				Rxcount = length - (end - buf);

				return d;
			case GOTCAN:
                                printf("Sender Canceled\n");
				//zperr1("Sender Canceled");
				return ZCAN;
			case TIMEOUT:
                                printf("TIMEOUT\n");
				//zperr1("TIMEOUT");
				return c;
			default:
				//garbitch(); 
				return c;
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
	return ERROR;
}

//void alrm(c) { longjmp(tohere, -1); }


/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
int noxrd7()
{
	register int c;

	for (;;) {
		if ((c = readline(Rxtimeout)) < 0)
			return c;
		switch (c &= 0177) {
		case XON:
		case XOFF:
			continue;
		default:
			if (Zctlesc && !(c & 0140))
				continue;
		case '\r':
		case '\n':
		case ZDLE:
			return c;
		}
                
	}
}

int zgeth1()
{
	register int c, n;

	if ((c = noxrd7()) < 0)
		return c;
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
		return ERROR;
	if ((c = noxrd7()) < 0)
		return c;
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
		return ERROR;
	c += (n<<4);
	return c;
}

/* Decode two lower case hex digits into an 8 bit byte value */
int zgethex()
{
	register int c;

	c = zgeth1();

	return c;
}

	/* NOTREACHED */
/*void openit(char *name, char *openmode){
	if (strcmp(name, "-"))                       //文件名字不带有- 即不为选项
		//fout = fopen(name, openmode);      //substitute with f_open in STM32
                f_open (fout, name, openmode);       //name 应该为文件路径
	else if (isatty(1))
		fout = fopen("stdout", "a");
	else
		fout = stdout;
}*/

void zputhex(register int c){
	static char	digits[] = "0123456789abcdef";
	sendline(digits[(c&0xF0)>>4]);
	sendline(digits[(c)&0xF]);
}


/*process incoming file information header
  returns 0 if success, other codes for errors
  or skip conditions*/
int procheader(char *name){
	register char *openmode, *p;
        static int dummy;
	 //printf("Hello, this is proheader program\r\n");
	/*set default parameters and overrides*/
	openmode = "w";
	Thisbinary = (!Rxascii) || Rxbinary;
	if(zconv == ZCBIN && Lzconv != ZCRESUM)
		Lzconv = zconv; // remote binary overrides
	if(Lzconv)
		zconv = Lzconv;
	if(Lzmanag)
		zmanag = Lzmanag;
	
	/*process ZMODEM remote file management request*/
	if(!Rxbinary && zconv == ZCNL)
		Thisbinary = 0;
	if(zconv == ZCBIN)
		Thisbinary = TRUE;
	else if (zmanag == ZMAPND)
		openmode = "a";
	
	Bytesleft = DEFBYTL;
	Filemode = 0;
	Modtime = 0L;
	
	if(!name || !*name){
                printf("name isn't a NULL pointer\r\n");
		return 0;
        }
	
	//set p address behind the name frame
	p = name + 1 + strlen(name);
	if (*p) {	/* file coming from Unix or DOS system */
		sscanf(p, "%ld%lo%o%lo%d%ld%d%d",
		  &Bytesleft, &Modtime, &Filemode,
		  &dummy, &Filesleft, &Totalleft, &dummy, &dummy);
		if (Filemode & UNIXFILE)
			++Thisbinary;
	}


	else {		/* File coming from CP/M system */
		for (p=name; *p; ++p)		/* change / to _ */
			if ( *p == '/')
				*p = '_';

		if ( *--p == '.')		/* zap trailing period 去掉句点 */
			*p = 0;
	}
	
	strcpy(Pathname, name);

	if(*name && f_stat(name, f) == 0){        //replaced with f_stat
		zmanag &= ZMMASK;
		if(zmanag == ZMPROT)
			goto skipfile;
                printf("Current %s is %ld of size %lo of last modified time\r\n", name, f->fsize, f->ftime);
		if(Thisbinary && zconv == ZCRESUM){
                        printf("In resuming interrupted file transfer\r\n");
                        rxbytes = f->fsize & ~511;    //清除低9位
			if (Bytesleft < rxbytes) {
				rxbytes = 0;
			rxbytes = 0;
			goto doopen;
		}
		else
			//openit(name, "r+");
                       verbose_open(&fout, mkpath(name), FA_READ| FA_WRITE | FA_CREATE_NEW);
                       
		if ( (&fout) == NULL)
                {
                                printf("The file doesn't exist, please check your path\r\n");
				return ZFERR;
                }
			if (f_lseek(&fout, rxbytes)) {
                                f_close(&fout);
				//closeit();
				return ZFERR;
			}
                        printf("Crash recovery at %ld\r\n", rxbytes);
			return 0;
        }	
		switch (zmanag & ZMMASK) {
		case ZMNEWL:
			if (Bytesleft > f->fsize)
				goto doopen;
		case ZMNEW:
			if ((f->ftime+1) >= Modtime)
				goto skipfile;
			goto doopen;
		case ZMCLOB:
		case ZMAPND:
			goto doopen;
		default:
			goto skipfile;
		}
	
		}
		else if (zmanag & ZMSKNOLOC){
skipfile:
		return ZSKIP;
		
	}
doopen:
	//openit(name, openmode);
         verbose_open(&fout, mkpath(name), FA_WRITE | FA_CREATE_NEW);
	//f_open (fout, name, FA_WRITE|FA_CREATE_ALWAYS);
	if ( (&fout) == NULL)
        {
                printf("In doopen branch, the file doesn't exist, please check your path\r\n");
		return ZFERR;
        }
        //printf("fout pointer is not NULL\r\n");
	return 0;
}


/*store long integer pos in Txhdr*/
void stohdr(long pos){
	Txhdr[ZP0] = pos;
	Txhdr[ZP1] = pos>>8;
	Txhdr[ZP2] = pos>>16;
	Txhdr[ZP3] = pos>>24;
}

/* Receive a binary style header (type and position) with 32 bit FCS */
int zrbhd32(char *hdr)
{
	register int c, n;
	register unsigned long crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype= c;
	crc = 0xFFFFFFFFL;
	crc = UPDC32(c, crc);

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
		*hdr = c;
	}
	
	for (n=4; --n >= 0;) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
	}
	
	if (crc != 0xDEBB20E3) {
                printf("%s\r\n",badcrc);
		//zperr1(badcrc);
		return ERROR;
	}
	Zmodem = 1;
	return Rxtype;
}



/* Receive a hex style header (type and position) */
int zrhhdr(char *hdr)
{
	register int c;
	register unsigned short crc;
	register int n;

	if ((c = zgethex()) < 0)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zgethex()) < 0)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		printf("%s\n",badcrc); 
                return ERROR;
	}
	c = readline(Rxtimeout);
	if (c < 0)
		return c;
	c = readline(Rxtimeout);
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	if (c < 0)
		return c;
	return Rxtype;
}

/* Receive a binary style header (type and position) */
int zrbhdr(char *hdr)
{
	register int c, n;
	register unsigned short crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n=Rxhlen; --n >= 0; ++hdr) {
		if ((c = zdlread()) & ~0377)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF) {
		printf("%s\n",badcrc);
		return ERROR;
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}


/*发送ZMODEM协议包头*/
void zshhdr(int len, int type, char *hdr){
	register int n;
	register unsigned short crc;
	
	sendline(ZPAD);
	sendline(ZPAD);
	sendline(ZDLE);
	sendline(ZHEX);
	zputhex(type);
	Crc32t = 0;
	
	crc = updcrc(type, 0);
	for (n=len; --n >= 0; ++hdr) {
		zputhex(*hdr); 
		crc = updcrc((0377 & *hdr), crc);
	}
	
	crc = updcrc(0,updcrc(0,crc));
	zputhex(((int)(crc>>8)));
	zputhex(crc);
	
	/* Make it printable on remote machine */
	sendline(015);
	sendline(0212);
	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != ZFIN && type != ZACK)
		sendline(021);
	printf("Have sent %s frametype\r\n", frametypes[type+FTOFFSET]);
	//加一个清除buffer的函数
}


int zgethdr(char *hdr){
	register int c, n, cancount;
	
	n = Zrwindow + Baudrate;
	Rxframeind = 0;
	Rxtype = 0;             /* Type of header received */
	
startover: 
	cancount = 5;
again:
	switch(c =readline(Rxtimeout)){
	case 021:   //17
	case 0221:   //145
		goto again;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	case CAN:   //030

gotcan:
		if(--cancount <= 0){
			c = ZCAN;
			goto fifi;
		}
	switch (c = readline(Rxtimeout)) {
		case TIMEOUT:
			goto again;
		case ZCRCW:
			switch (readline(Rxtimeout)) {
			case TIMEOUT:
				c = ERROR; 
				goto fifi;
			case RCDO:
				goto fifi;
			default:
				goto agn2;
			}
		case RCDO:
			goto fifi;
		default:
			break;
		case CAN:
			if (--cancount <= 0) {
				c = ZCAN; 
				goto fifi;
			}
			goto again;
		}
	/* **** FALL THRU TO **** */
	default:
agn2:
		if ( --n == 0) {
			c = GCOUNT;  
			goto fifi;
		}
		goto startover;
	case ZPAD:		/* This is what we want. */
          {
                printf("We get ZPAD\r\n");
		break;
          }
	}
	cancount = 5;
splat:
	switch (c = noxrd7()) {
	case ZPAD:
		goto splat;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	default:
		goto agn2;
	case ZDLE:		/* This is what we want. */
          {
                printf("We get ZDLE\r\n");
		break;
          }
	}


	Rxhlen = 4;		/* Set default length */
	c = readline(Rxtimeout);
        Rxframeind = c;
       // printf("Got %s\r\n", frametypes[c+FTOFFSET]);
	switch (c) {
	case ZVBIN32:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 1;  
		c = zrbhd32(hdr); 
		break;
	case ZBIN32:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 1;  
		c = zrbhd32(hdr); 
		break;
	case ZVBINR32:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 2;  
		c = zrbhd32(hdr); 
		break;
	case ZBINR32:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 2;  
		c = zrbhd32(hdr); 
		break;
	case RCDO:
	case TIMEOUT:
		goto fifi;
	case ZVBIN:
		if ((Rxhlen = c = zdlread()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 0;  
		c = zrbhdr(hdr); 
		break;
	case ZBIN://0x41  This is what we want 
                printf("In ZBIN\r\n");
		if (Usevhdrs)
			goto agn2;
		Crc32r = 0;  
		c = zrbhdr(hdr); 
		break;
	case ZVHEX:
		if ((Rxhlen = c = zgethex()) < 0)
			goto fifi;
		if (c > ZMAXHLEN)
			goto agn2;
		Crc32r = 0;  
		c = zrhhdr(hdr); 
		break;
	case ZHEX:
		if (Usevhdrs)
			goto agn2;
		Crc32r = 0;  
		c = zrhhdr(hdr); 
		break;
	case CAN:
		goto gotcan;
	default:
		goto agn2;
	}
	for (n = Rxhlen; ++n < ZMAXHLEN; )	/* Clear unused hdr bytes */
		hdr[n] = 0;
	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos<<8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos<<8) + (hdr[ZP0] & 0377);
fifi:
        //printf("In fifi, the received header is %s\r\n", frametypes[c+FTOFFSET] );
	switch (c) {
	case GOTCAN:
		c = ZCAN;
	/* **** FALL THRU TO **** */
	case ZNAK:
	case ZCAN:
	case ERROR:
	case TIMEOUT:
	case RCDO:
	case GCOUNT:
		printf("Got %s\r\n", frametypes[c+FTOFFSET]);
	/* **** FALL THRU TO **** */
	default:
		if (c >= -4 && c <= FRTYPES)
                  printf("In fifi,zgethdr: the frametype:%c the received header length:%d\r\n the headertype: %s the received file position: %lx", Rxframeind, Rxhlen, frametypes[c+FTOFFSET], Rxpos);
		else
                  printf("In fifi, zgethdr: the frametype:%c the received header length:%d \r\n the received file position: %lx", Rxframeind, c, Rxpos);
                
	}
	
	/* Use variable length headers if we got one */
	
	if (c >= 0  && Rxframeind & 040)
		Usevhdrs = 1;
	
	return c;
}

long rclhdr(register char *hdr)
{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return l;
}


/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */
int tryz(){
          register int  c, n;
	  register int cmdzack1flg;
  
        //printf("In tryz function \r\n");
	for (n=15; --n>=0; ) {
		/* Set buffer length (0) and capability flags */

		stohdr(0L);

		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO;
                
		if (Zctlesc)
			Txhdr[ZF0] |= TESCCTL;
		Txhdr[ZF0] |= CANRLE;
		Txhdr[ZF1] = CANVHDR;
                
		/* tryzhdrtype may == ZRINIT */
		zshhdr(4, tryzhdrtype, Txhdr);
                
		if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
			tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */
again:
		switch (zgethdr(Rxhdr)) {
		case ZRQINIT:
			if (Rxhdr[ZF3] & 0x80)
				Usevhdrs = 1;	/* we can var header */
			continue;
		case ZEOF:
			continue;
		case TIMEOUT:
			continue;
		case ZFILE:
			zconv = Rxhdr[ZF0];
			zmanag = Rxhdr[ZF1];
			ztrans = Rxhdr[ZF2];
			if (Rxhdr[ZF3] & ZCANVHDR)
				Usevhdrs = TRUE;
			tryzhdrtype = ZRINIT;
			c = zrdata(secbuf, 1024);
			if (c == GOTCRCW)
				return ZFILE;
			zshhdr(4,ZNAK, Txhdr);
			goto again;
		case ZSINIT:
			Zctlesc = TESCCTL & Rxhdr[ZF0];
			if (zrdata(Attn, ZATTNLEN) == GOTCRCW) {
				stohdr(1L);
				zshhdr(4,ZACK, Txhdr);
				goto again;
			}
			zshhdr(4,ZNAK, Txhdr);
			goto again;
		case ZFREECNT:
			stohdr(getfree());
			zshhdr(4,ZACK, Txhdr);
			goto again;
		case ZCOMMAND:

			cmdzack1flg = Rxhdr[ZF0];
			if (zrdata(secbuf, 1024) == GOTCRCW) {
				//void exec2();

				if (cmdzack1flg & ZCACK1)
					stohdr(0L);
				else
					//stohdr((long)sys2(secbuf));
				//purgeline();	/* dump impatient questions */
				do {
					zshhdr(4,ZCOMPL, Txhdr);
				}
				while (++errors<20 && zgethdr(Rxhdr) != ZFIN);
				//ackbibi();
				/*if (cmdzack1flg & ZCACK1)
					exec2(secbuf);*/
				return ZCOMPL;
			}
			zshhdr(4,ZNAK, Txhdr); goto again;
		case ZCOMPL:
			goto again;
		default:
			continue;
		case ZFIN:
			//ackbibi(); 
                        return ZCOMPL;
		case ZCAN:
			return ERROR;
		}
	}
	return 0;

}


/*receive 1 file with ZMODEM protocol*/
int rzfile(char* secbuf)
{                
        printf("Hello, this is rzfile program\r\n");
        register int c,n;
	Eofseen = FALSE;
	n = 20;
	rxbytes = 0l;
		
	if(c = procheader(secbuf)){
		return (tryzhdrtype = c);
	}

	for(;;){
                //printf("Send ZRINIT success\r\n");
		stohdr(rxbytes);
		zshhdr(4, ZRPOS, Txhdr);
		
nxthdr:
                c = zgethdr(Rxhdr);
                printf("In nxthdr, got %s\r\n", frametypes[c+FTOFFSET]);
		switch(c){
		 	
		default:
			//打印wrong header
			if(--n < 0){
				printf("rzfile: Wrong header %d\r\n", c);
				return ERROR;
			}
			continue;
		case ZCAN:
                      printf("Sender CANcelled\r\n");
			return ERROR;
                case ZNAK:
                      if ( --n < 0) {
				printf("rzfile: got ZNAK\r\n");
				return ERROR;
			}
			continue;
		case TIMEOUT:
		    if ( --n < 0) {
				printf("rzfile: TIMEOUT\r\n");
				return ERROR;
			}
			continue;
		
		case ZFILE:
			zrdata(secbuf, 1024);
			continue;
		
		case ZEOF:
			if(rclhdr(Rxhdr) != rxbytes){
				/*
				 * Ignore eof if it's at wrong place - force
				 *  a timeout because the eof might have gone
				 *  out before we sent our zrpos.
				 */
				 errors = 0;
				 goto nxthdr;
			}
		
		if(f_close(&fout) != 0){
			tryzhdrtype = ZFERR;
                        printf("Error closing file\r\n");
			return ERROR;
		}
		return c;
		
		case ERROR:
		if ( --n < 0) {
				printf("Persistent CRC or other ERROR\r\n");
				return ERROR;
			}
	    
		zmputs(Attn);
		continue;
		
		case ZSKIP:
		Modtime = 1;
		f_close(&fout);
		printf("Sender SKIPPED file\r\n");
		return c;
		
		case ZDATA:
			if (rclhdr(Rxhdr) != rxbytes) {
				if ( --n < 0) {
					printf("Data has bad addr\r\n");
					return ERROR;
				}
			zmputs(Attn);
			continue;
		}
		
moredata:
		switch (c = zrdata(secbuf, 1024)){
		
		case ZCAN:
                        printf("Sender CANcelled\r\n");
				return ERROR;
				
		case ERROR: //CRC error
			if ( --n < 0) {
				printf("Persistent CRC or other ERROR\n");
				return ERROR;
			}
				zmputs(Attn);
			continue;
		
		case TIMEOUT:
			if ( --n < 0) {
				printf("TIMEOUT\n");
				return ERROR;
			}
			continue;
		
		case GOTCRCW:
			n = 20;
			putsec(secbuf, Rxcount);
			
		    rxbytes += Rxcount;
			stohdr(rxbytes);
			sendline(XON);
			zshhdr(4,ZACK, Txhdr);
			goto nxthdr;	
		
		case GOTCRCQ:
			n = 20;
			putsec(secbuf, Rxcount);
			
			rxbytes += Rxcount;
			stohdr(rxbytes);
			zshhdr(4,ZACK, Txhdr);
			goto moredata;
			
		case GOTCRCG:
			n = 20;
			putsec(secbuf, Rxcount);
			
			rxbytes += Rxcount;
			goto moredata;
			
		case GOTCRCE:
			n = 20;
			putsec(secbuf, Rxcount);
			
			rxbytes += Rxcount;
			goto nxthdr;
		}
	  }
	}
}


/*
 * Receive 1 or more files with ZMODEM protocol
 */
int rzfiles()
{
	register int c;
        printf("In rzfiles function\r\n");
	for (;;) {
		switch (c = rzfile(secbuf)) {
		case ZEOF:
		case ZSKIP:
		case ZFERR:
			switch (tryz()) {
			case ZCOMPL:
				return OK;
			default:
				return ERROR;
			case ZFILE:
				break;
			}
			continue;
		default:
			return c;
		case ERROR:
			return ERROR;
		}
	}
	/* NOTREACHED */
}


/*
 * Wcgetsec fetches a Ward Christensen type sector.
 * Returns sector number encountered or ERROR if valid sector not received,
 * or CAN CAN received
 * or WCEOT if eot sector
 * time is timeout for first char, set to 4 seconds thereafter
 ***************** NO ACK IS SENT IF SECTOR IS RECEIVED OK **************
 *    (Caller must do that when he is good and ready to get next sector)
 */

int wcgetsec(char *rxbuf, int maxtime)  //传进来第一个参数是setbuf
{
	register int checksum, wcj, firstch;
	register unsigned short oldcrc;
	register char *p;
	int sectcurr;

	for (Lastrx=errors=0; errors<RETRYMAX; errors++) {
		/*判断包头是SOH还是STX从而判断数据长度*/
		if ((firstch=readline(maxtime))==STX) {
			Blklen=1024; 
			goto get2;
		}
		if (firstch==SOH) {
			Blklen=128;
get2:
			sectcurr=readline(1);
			if ((sectcurr+(oldcrc=readline(1)))==0377) {
				oldcrc=checksum=0;
				for (p=rxbuf,wcj=Blklen; --wcj>=0; ) {
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if ((firstch=readline(1)) < 0)
					goto bilge;
				if (Crcflg) {
					oldcrc=updcrc(firstch, oldcrc);
					if ((firstch=readline(1)) < 0)
						goto bilge;
					oldcrc=updcrc(firstch, oldcrc);
					if (oldcrc & 0xFFFF)
						printf( "CRC\r\n");
					else {
						Firstsec=FALSE;
						return sectcurr;   //正常返回
					}
				}
				else if (((checksum-firstch)&0377)==0) {
					Firstsec=FALSE;
					return sectcurr;    //正常返回
				}
				else
					printf( "Checksum\r\n");
			}
			else
				printf("Sector number garbled\r\n");
		}
		/* make sure eot really is eot and not just mixmash */
		else if (firstch==EOT && Lleft==0)
			return WCEOT;
		else if (firstch==CAN) {
			if (Lastrx==CAN) {
				printf( "Sender CANcelled\r\n");
				return ERROR;
			} else {
				Lastrx=CAN;
				continue;
			}
		}
		else if (firstch==TIMEOUT) {
			if (Firstsec)
				goto humbug;
bilge:
			printf( "TIMEOUT\r\n");
		}
		else
			printf( "Got 0%o sector header\r\n", firstch);

humbug:
		Lastrx=0;
		while(readline(1)!=TIMEOUT)
			;
		if (Firstsec) {
			sendline(Crcflg?WANTCRC:NAK);  
                        //flushmo();
			Lleft=0;	/* Do read next time ... */
		} else {
			maxtime=40; sendline(NAK);  
                        //flushmo();
			Lleft=0;	/* Do read next time ... */
		}
	}
	/* try to stop the bubble machine. */
	canit();
	return ERROR;
}



/*
 * Fetch a pathname from the other end as a C ctyle ASCIZ string.
 * Length is indeterminate as long as less than Blklen
 * A null string represents no more files (YMODEM)
 */
int wcrxpn(char *rpn)	/* receive a pathname */
{
	register int c;

	//purgeline();

et_tu:
	Firstsec = TRUE; 
	Eofseen = FALSE;
	sendline(Crcflg?WANTCRC:NAK);  
	Lleft=0;	/* Do read next time ... */
	switch (c = wcgetsec(rpn, 100)) {
	case WCEOT:
		printf( "Pathname fetch returned %d\r\n", c);
		sendline(ACK);  
		Lleft=0;	/* Do read next time ... */
		readline(1);
		goto et_tu;
	case 0:
		sendline(ACK);  
                return OK;
	default:
		return ERROR;
	}
}

/*
 * Adapted from CMODEM13.C, written by
 * Jack M. Wierda and Roderick W. Hart
 */

int wcrx()
{
	register int sectnum, sectcurr;
	register char sendchar; /*data to be sent*/
	int cblklen;			/* bytes to dump this block */

	Firstsec=TRUE;
	sectnum=0;
	Eofseen=FALSE;
	sendchar=Crcflg?WANTCRC:NAK;   /*如果CRCflg为真 则启用传输sendchar=NAK*/

	for (;;) {
		sendline(sendchar);	/* send it now, we're ready! */
		//flushmo();
		Lleft=0;	/* Do read next time ... number of characters in linbuf linbuf为剩下字节缓冲*/ 
		sectcurr=wcgetsec(secbuf, (sectnum&0177)?50:130);  //低7位为1
		if (sectcurr==(sectnum+1 &0377)) {
			sectnum++;
			cblklen = Bytesleft>Blklen ? Blklen:Bytesleft;
			if (putsec(secbuf, cblklen)==ERROR)
				return ERROR;
			if ((Bytesleft-=cblklen) < 0)
				Bytesleft = 0;
			sendchar=ACK;
		}
		else if (sectcurr==(sectnum&0377)) {
			printf( "Received dup Sector\r\n");
			sendchar=ACK;
		}
		else if (sectcurr==WCEOT) {
			if (f_close(&fout) != 0)
				return ERROR;
			sendline(ACK); 
			//flushmo();
			Lleft=0;	/* Do read next time ... */
			return OK;  //成功返回
		}
		else if (sectcurr==ERROR)
			return ERROR;
		else {
			printf( "Sync Error\r\n");
			return ERROR;
		}
	}
	/* NOTREACHED */
}





/************IMPORTANT RZ  External Function************/
//int rz(char* fname)                 //传递参数还没有确定
int rz(void)
{
        register int c;
        printf("In rz function\r\n");
        if (c = tryz()) {
			if (c == ZCOMPL)
				return OK;
			if (c == ERROR)
				
				goto fubar;
			c = rzfiles();
			if (c)
				goto fubar;
		} else {
			for (;;) {
				if (wcrxpn(secbuf)== ERROR)
					goto fubar;
				if (secbuf[0]==0)
					return OK;
				if (procheader(secbuf))
					goto fubar;
				if (wcrx()==ERROR)
					goto fubar;
			}
		}
	
          return OK;
		
fubar:
	printf("Error automatically go to fubar\r\n");
	Modtime = 1;
	if ((&fout) == NULL)
		f_close(&fout);

	return ERROR;
			
		
}




uart_config_t zmodem_uart_config = {
	.USART_BaudRate = Z_BAUD,
	.USART_WordLength = USART_WordLength_8b,
	.USART_StopBits = USART_StopBits_1,
	.USART_Parity = USART_Parity_No,
	.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	.USART_HardwareFlowControl = USART_HardwareFlowControl_None
};

void zinit()
{       
        uart_init(SERIAL_PORT, &zmodem_uart_config);
        uart_open(SERIAL_PORT, O_RDWR);
        
}


void sz(char* fname)
{

}

void zclose()
{
        uart_close(SERIAL_PORT);
}


void test(){
        
        char a[] = "abc";
        register int n;
        n  = uart_write(SERIAL_PORT, (uint8_t*)a, 3);
                  
                  if(n >0) {
                  printf("Write data success, now try to grap the written data\r\n");
                  }
                  //memset(a, 0, strlen(a));
                  if(uart_available(SERIAL_PORT))
                {
                  printf("STM32 USART communication success, waiting for received data\r\n");
                  
                  n = uart_read(SERIAL_PORT, (uint8_t*)a, 3);
                  printf("The size of received is %d, the received data is %s\r\n", n, a);
                  
                }
}



