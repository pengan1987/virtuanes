/*******************************************************************************
 * NES Mapper for BBK
 *
 *  Author:  <fanoble@yeah.net>
 *
 *  Create:   2014-06-24, by fanoble
 *******************************************************************************
 */

#ifdef BBKE

class	MapperBBK : public Mapper
{
public:
	MapperBBK( NES* parent ) : Mapper(parent) {}

	void	Reset();

	// 8000 - FFFF
	BOOL	ReadHigh( WORD addr, LPBYTE pdata );
	void	Write( WORD addr, BYTE data );

	// 4100 - 7FFF
	BYTE	ReadLow ( WORD addr );
	void	WriteLow( WORD addr, BYTE data );

	void	FdcHardReset(void);
	void	FdcSoftReset(void);

	static void FdcNop(MapperBBK* thiz);
	static void FdcReadTrack(MapperBBK* thiz);
	static void FdcSpecify(MapperBBK* thiz);
	static void FdcSenseDriveStatus(MapperBBK* thiz);
	static void FdcWriteData(MapperBBK* thiz);
	static void FdcReadData(MapperBBK* thiz);
	static void FdcRecalibrate(MapperBBK* thiz);
	static void FdcSenseIntStatus(MapperBBK* thiz);
	static void FdcWriteDeletedData(MapperBBK* thiz);
	static void FdcReadID(MapperBBK* thiz);
	static void FdcReadDeletedData(MapperBBK* thiz);
	static void FdcFormatTrack(MapperBBK* thiz);
	static void FdcSeek(MapperBBK* thiz);
	static void FdcScanEqual(MapperBBK* thiz);
	static void FdcScanLowOrEqual(MapperBBK* thiz);
	static void FdcScanHighOrEqual(MapperBBK* thiz);

	typedef struct tagFDC_CMD_DESC
	{
		BYTE	bWLength;
		BYTE	bRLength;
		void	(*pFun)(MapperBBK*);
	} FDC_CMD_DESC;

protected:
private:
	// for FDC
	BOOL	bFdcIrq;
	BOOL	bFdcHwReset;
	BOOL	bFdcSoftReset;

	BOOL	bFdcDmaInt;
	UINT	nFdcDrvSel;
	UINT	nFdcMotor;

#define FDC_MS_BUSYS0		0x01	// FDD0 in SEEK mode
#define FDC_MS_BUSYS1		0x02	// FDD1 in SEEK mode
#define FDC_MS_BUSYS2		0x04	// FDD2 in SEEK mode
#define FDC_MS_BUSYS3		0x08	// FDD3 in SEEK mode
#define FDC_MS_BUSYRW		0x10	// Read or Write in progress
#define FDC_MS_EXECUTION	0x20	// Execution Mode
#define FDC_MS_DATA_IN		0x40	// Data input or output
#define FDC_MS_RQM			0x80	// Request for Master, Ready
	BYTE	nFdcMainStatus;

#define FDC_S0_US0			0x01	// Unit Select 0
#define FDC_S0_US1			0x02	// Unit Select 1
#define FDC_S0_HD			0x04	// Head Address
#define FDC_S0_NR			0x08	// Not Ready
#define FDC_S0_EC			0x10	// Equipment Check
#define FDC_S0_SE			0x20	// Seek End
#define FDC_S0_IC0			0x40	// Interrupt Code
#define FDC_S0_IC1			0x80	// NT/AT/IC/XX

#define FDC_S1_MA			0x01	// Missing Address Mark
#define FDC_S1_NW			0x02	// Not Writable
#define FDC_S1_ND			0x04	// No Data
#define FDC_S1_OR			0x10	// Over Run
#define FDC_S1_DE			0x20	// Data Error
#define FDC_S1_EN			0x80	// End of Cylinder

#define FDC_S2_MD			0x01	// Missing Address Mark in Data Field
#define FDC_S2_BC			0x02	// Bad Cylinder
#define FDC_S2_SN			0x04	// Scan Not Satisfied
#define FDC_S2_SH			0x08	// Scan Equal Hit
#define FDC_S2_WC			0x10	// Wrong Cylinder
#define FDC_S2_DD			0x20	// Data Error in Data Field
#define FDC_S2_CM			0x40	// Control Mark

#define FDC_S3_US0			0x01	// Unit Select 0
#define FDC_S3_US1			0x02	// Unit Select 1
#define FDC_S3_HD			0x04	// Side Select
#define FDC_S3_TS			0x08	// Two Side
#define FDC_S3_T0			0x10	// Track 0
#define FDC_S3_RY			0x20	// Ready
#define FDC_S3_WP			0x40	// Write Protect
#define FDC_S3_FT			0x80	// Fault
	BYTE	nFDCStatus[4];

#define FDC_CC_MASK						0x1F
#define FDC_CF_MT						0x80
#define FDC_CF_MF						0x40
#define FDC_CF_SK						0x20

#define FDC_CC_READ_TRACK				0x02
#define FDC_CC_SPECIFY					0x03
#define FDC_CC_SENSE_DRIVE_STATUS		0x04
#define FDC_CC_WRITE_DATA				0x05
#define FDC_CC_READ_DATA				0x06
#define FDC_CC_RECALIBRATE				0x07
#define FDC_CC_SENSE_INTERRUPT_STATUS	0x08
#define FDC_CC_WRITE_DELETED_DATA		0x09
#define FDC_CC_READ_ID					0x0A
#define FDC_CC_READ_DELETED_DATA		0x0C
#define FDC_CC_FORMAT_TRACK				0x0D
#define FDC_CC_SEEK						0x0F
#define FDC_CC_SCAN_EQUAL				0x11
#define FDC_CC_SCAN_LOW_OR_EQUAL		0x19
#define FDC_CC_SCAN_HIGH_OR_EQUAL		0x1D
	BYTE	bFdcCycle;
	BYTE	bFdcCommands[10];
	BYTE	bFdcResults[8];
	const FDC_CMD_DESC* pFdcCmd;

#define FDC_PH_IDLE						0
#define FDC_PH_COMMAND					1
#define FDC_PH_EXECUTION				2
#define FDC_PH_RESULT					3
	BYTE	bFdcPhase;

	BYTE	nFdcCylinder;
	BYTE	nPageCD;
	BYTE	nPageEF;

	LPBYTE	pFdcDataPtr;

	// for ROM and RAM
//	BYTE	BIOS[0x4000];
	LPBYTE	BIOS;
	LPBYTE	DISK;
	BYTE	DRAM[1 * 1024 * 1024];
};
#endif

/*******************************************************************************
                           E N D  O F  F I L E
*******************************************************************************/
