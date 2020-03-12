/*******************************************************************************
 * NES Mapper for BBK
 *
 *  Author:  <fanoble@yeah.net>
 *
 *  Create:   2014-06-24, by fanoble
 *******************************************************************************
 */

#ifdef BBKE

static const MapperBBK::FDC_CMD_DESC FdcCmdTable[32] =
{
	/* 0x00 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x01 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x02 */ { 9, 7, MapperBBK::FdcReadTrack },
	/* 0x03 */ { 3, 0, MapperBBK::FdcSpecify },
	/* 0x04 */ { 2, 1, MapperBBK::FdcSenseDriveStatus },
	/* 0x05 */ { 9, 7, MapperBBK::FdcWriteData },
	/* 0x06 */ { 9, 7, MapperBBK::FdcReadData },
	/* 0x07 */ { 2, 0, MapperBBK::FdcRecalibrate },
	/* 0x08 */ { 1, 2, MapperBBK::FdcSenseIntStatus },
	/* 0x09 */ { 9, 7, MapperBBK::FdcWriteDeletedData },
	/* 0x0A */ { 2, 7, MapperBBK::FdcReadID },
	/* 0x0B */ { 1, 1, MapperBBK::FdcNop },
	/* 0x0C */ { 9, 7, MapperBBK::FdcReadDeletedData },
	/* 0x0D */ { 6, 7, MapperBBK::FdcFormatTrack },
	/* 0x0E */ { 1, 1, MapperBBK::FdcNop },
	/* 0x0F */ { 3, 0, MapperBBK::FdcSeek },
	/* 0x10 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x11 */ { 9, 7, MapperBBK::FdcScanEqual },
	/* 0x12 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x13 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x14 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x15 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x16 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x17 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x18 */ { 1, 1, MapperBBK::FdcNop },
	/* 0x19 */ { 9, 7, MapperBBK::FdcScanLowOrEqual },
	/* 0x1A */ { 1, 1, MapperBBK::FdcNop },
	/* 0x1B */ { 1, 1, MapperBBK::FdcNop },
	/* 0x1C */ { 1, 1, MapperBBK::FdcNop },
	/* 0x1D */ { 9, 7, MapperBBK::FdcScanHighOrEqual },
	/* 0x1E */ { 1, 1, MapperBBK::FdcNop },
	/* 0x1F */ { 1, 1, MapperBBK::FdcNop },
};

void MapperBBK::Reset()
{
	DISK = nes->rom->GetDISK();

	pFdcDataPtr = DISK;

	// BIOS fixed to Page FE
	BIOS = DRAM + 0x78000;

	memset(DRAM, 0, sizeof(DRAM));

	// 4xxx - 7xxx
	SetPROM_Bank(2, BIOS + 0x0000, BANKTYPE_RAM);
	SetPROM_Bank(3, BIOS + 0x2000, BANKTYPE_RAM);

	// 8xxx - Bxxx
	SetPROM_Bank(4, DRAM + 0x0000, BANKTYPE_DRAM);
	SetPROM_Bank(5, DRAM + 0x2000, BANKTYPE_DRAM);

	// Cxxx - Fxxx
	SetPROM_16K_Bank(6, 7);

	SetVROM_Bank( 8, VRAM + 0x0000, BANKTYPE_VRAM);
	SetVROM_Bank( 9, VRAM + 0x0400, BANKTYPE_VRAM);
	SetVROM_Bank(10, VRAM + 0x0800, BANKTYPE_VRAM);
	SetVROM_Bank(11, VRAM + 0x0C00, BANKTYPE_VRAM);

	nPageCD = 0;
	nPageEF = 0;

	// FDC
	bFdcIrq = FALSE;
	bFdcHwReset = FALSE;
	bFdcSoftReset = FALSE;

	bFdcDmaInt = FALSE;
	nFdcDrvSel = 0;
	nFdcMotor = 0;
	nFdcMainStatus = FDC_MS_RQM;

	nFDCStatus[0] = 0;
	nFDCStatus[1] = 0;
	nFDCStatus[2] = 0;
	nFDCStatus[3] = 0;

	bFdcCycle = 0;
	bFdcPhase = FDC_PH_IDLE;

	nFdcCylinder = 0;

	// set ExPad
	nes->pad->SetExController(PAD::EXCONTROLLER_SUPOR_KEYBOARD);
}

// 8000 - FFFF
BOOL MapperBBK::ReadHigh(WORD addr, LPBYTE pdata)
{
	if (addr < 0xFF00 ||
		addr > 0xFFEF)
		return FALSE;

	switch (addr)
	{
		case 0xFF80:	// FDCDMADackIO
			*pdata = *pFdcDataPtr++;
			return TRUE;
		case 0xFF88:	// FDCDMATcIO
			*pdata = *pFdcDataPtr++;
			return TRUE;
		case 0xFF90:	// FDCDRQPortI/FDCCtrlPortO
			// I: D6 : FDC DRQ
			*pdata = 0x40;
			return TRUE;
		case 0xFF98:	// FDCIRQPortI/FDCDMADackIO
			// I: D6 : IRQ
			if (bFdcIrq)
				*pdata = 0x40;
			else
				*pdata = 0;
			return TRUE;

		case 0xFFA0:	// FDCResetPortO/FDCStatPortI
			// I: D7 : FDC ready
			// I: D6 : FDC dir
			// I: D5 : FDC busy
			*pdata = nFdcMainStatus;
			return TRUE;
		case 0xFFA8:	// FDCDataPortIO
			*pdata = bFdcResults[bFdcCycle];
			bFdcCycle++;
			if (bFdcCycle == pFdcCmd->bRLength)
			{
				// prepare for next command
				bFdcCycle = 0;
				bFdcPhase = FDC_PH_IDLE;

				nFdcMainStatus &= ~FDC_MS_DATA_IN;
				nFdcMainStatus |= FDC_MS_RQM;
			}
			return TRUE;
		case 0xFFB8:	// FDCChangePortI/FDCSpeedPortO
			// I: D7 : Disk changed
			*pdata = 0;
			return TRUE;

		default:
			DEBUGOUT("Read %02X\n", addr);
			break;
	}

	return FALSE;
}

void MapperBBK::Write(WORD addr, BYTE data)
{
	if (addr < 0xFF00)
	{
		CPU_MEM_BANK[addr >> 13][addr & 0x1FFF] = data;
		return;
	}

	switch (addr)
	{
		case 0xFF00:	// KebBoardLEDPort
			break;

		case 0xFF01:	// VideoCtrlPort
			// D3 : C000-FFFF map to DRAM/-ROM
			// D2 : INT count/-load
			// D[1:0] : 6116's AV10 connection
			//          00: AV10
			//          01: AV11
			//          10: AV0
			//          11: AV1
			if (data & 8)
			{
				// map C000-FFF to DRAM
				int nOffset;

				nOffset = nPageCD * 0x2000;
				nOffset &= 0x7FFFF;	// truncate to 512K
				SetPROM_Bank(6, DRAM + nOffset, BANKTYPE_DRAM);

				nOffset = nPageEF * 0x2000;
				nOffset &= 0x7FFFF;	// truncate to 512K
				SetPROM_Bank(7, DRAM + nOffset, BANKTYPE_DRAM);
			}
			else
			{
				// map C000-FFF to ROM
				SetPROM_16K_Bank(6, 7);
			}
			break;

//		case 0xFF02:	// IntCountPortL
//			// D[4:0] : counter
//			break;

		case 0xFF03:	// VideoDataPort0
			data &= 0x0F;	// 32K
			SetVROM_Bank(0, VRAM + data * 0x0800 + 0x0000, BANKTYPE_VRAM);
			SetVROM_Bank(1, VRAM + data * 0x0800 + 0x0400, BANKTYPE_VRAM);
			break;

		case 0xFF04:	// DRAMPagePort
			data &= 0x1F;	//  512K
			SetPROM_Bank(4, DRAM + data * 0x4000 + 0x0000, BANKTYPE_DRAM);
			SetPROM_Bank(5, DRAM + data * 0x4000 + 0x2000, BANKTYPE_DRAM);
			break;

		// code patch
		case 0xFF0A:
		case 0xFF12:
		case 0xFF1A:
		case 0xFF22:
		case 0xFF2B:
			CPU_MEM_BANK[7][addr & 0x1FFF] = data;
			break;

		case 0xFF0B:	// VideoDataPort1
			data &= 0x0F;	// 32K
			SetVROM_Bank(2, VRAM + data * 0x0800 + 0x0000, BANKTYPE_VRAM);
			SetVROM_Bank(3, VRAM + data * 0x0800 + 0x0400, BANKTYPE_VRAM);
			break;

		case 0xFF13:	// VideoDataPort2
			data &= 0x0F;	// 32K
			SetVROM_Bank(4, VRAM + data * 0x0800 + 0x0000, BANKTYPE_VRAM);
			SetVROM_Bank(5, VRAM + data * 0x0800 + 0x0400, BANKTYPE_VRAM);
			break;


		case 0xFF1B:	// VideoDataPort3
			data &= 0x0F;	// 32K
			SetVROM_Bank(6, VRAM + data * 0x0800 + 0x0000, BANKTYPE_VRAM);
			SetVROM_Bank(7, VRAM + data * 0x0800 + 0x0400, BANKTYPE_VRAM);
			break;

		case 0xFF24:	// DRAMPagePortCD
			nPageCD = data;
			break;
		case 0xFF2C:	// DRAMPagePortEF
			nPageEF = data;
			break;

		case 0xFF10:	// SoundPort0/SpeakInitPort
			break;
		case 0xFF18:	// SoundPort1/SpeakDataPort
			break;

		case 0xFF40:	// PCDaCtPortO/PCCDataPort
			// D4 : Signal to PC's ERROR
			// D[3:0] : Output PC data
			break;
		case 0xFF48:	// PCCtrlPortI/PCCStatsPortI/PCDataPortIH
			// D[7:4] : PC input data
			// D3 : Signal of PC's pin STB
			break;
		case 0xFF50:	// PCDataPortIL/PCCCtrlPoutO
			// D[3:0] : PC input data
			break;

		case 0xFF80:	// FDCDMADackIO
			data = data;
			break;
		case 0xFF88:	// FDCDMATcIO
			data = data;
			break;
		case 0xFF90:	// FDCDRQPortI/FDCCtrlPortO
			// O: D5 : Drv B motor
			// O: D4 : Drv A motor
			// O: D3 : Enable INT and DMA
			// O: D2 : not FDC Reset
			// O: D[1:0] : Drv sel

			bFdcDmaInt = (data & 8) ? TRUE : FALSE;
			nFdcDrvSel = data & 3;
			nFdcMotor = data >> 4;

			if (data & 4)
			{
				if (bFdcSoftReset)
				{
					FdcSoftReset();

					bFdcSoftReset = FALSE;

					// irq after soft reset
					if (0 == nFdcDrvSel)
						bFdcIrq = TRUE;		// Drv A Only
					else
						bFdcIrq = FALSE;
				}
			}
			else
			{
				if (!bFdcSoftReset)
				{
					bFdcSoftReset = TRUE;
					bFdcIrq = FALSE;
				}
			}

			break;
		case 0xFF98:	// FDCIRQPortI/FDCDMADackIO
			// I: D6 : IRQ
			data = data;
			break;
		case 0xFFA0:	// FDCResetPortO/FDCStatPortI
			// O: D6 : FDC pin reset
			if (data & 0x40)
			{
				if (!bFdcHwReset)
				{
					bFdcHwReset = TRUE;
					bFdcIrq = FALSE;
				}
			}
			else
			{
				if (bFdcHwReset)
				{
					FdcHardReset();
					bFdcHwReset = FALSE;
				}
			}
			break;
		case 0xFFA8:	// FDCDataPortIO
			switch (bFdcPhase)
			{
				case FDC_PH_EXECUTION:
				case FDC_PH_RESULT:
					// ERROR
					break;
				case FDC_PH_IDLE:
				default:
					bFdcCycle = 0;
					bFdcPhase = FDC_PH_COMMAND;
					pFdcCmd = &FdcCmdTable[data & FDC_CC_MASK];
					// fall through
				case FDC_PH_COMMAND:
					bFdcCommands[bFdcCycle] = data;
					bFdcCycle++;
					if (bFdcCycle == pFdcCmd->bWLength)
					{
						bFdcPhase = FDC_PH_EXECUTION;
//						nFdcMainStatus &= ~FDC_MS_RQM;

						pFdcCmd->pFun(this);

						// prepare for reading
						if (pFdcCmd->bRLength)
						{
							nFdcMainStatus |= FDC_MS_DATA_IN;
							bFdcPhase = FDC_PH_RESULT;
						}
						else
						{
							bFdcPhase = FDC_PH_IDLE;
						}

						bFdcCycle = 0;
					}
					break;
			}
			break;
		case 0xFFB8:	// FDCChangePortI/FDCSpeedPortO
			// I: D7 : Disk changed
			// O: D[1:0] : 00 500kbps(1.2M, 1.44M)
			//             01 300kbps(360K) 
			//             10 250kbps(720K)
			data = data;
			break;

		case 0xFF09:
		case 0xFF17:
			data = data;
			break;

		default:
			DEBUGOUT("Write %02X -> [%04X]\n", data, addr);
			break;
	}
}

// 4100 - 7FFF
BYTE MapperBBK::ReadLow(WORD addr)
{
	BYTE data = 0;

	if (addr < 0x4400)
	{
		// 4100 - 43FF, IO for system
		data = BIOS[addr - 0x4000];
	}
	else if (addr < 0x5800)
	{
		// 4400 - 57FF, IO for ROM/RAM
		data = BIOS[addr - 0x4000];
	}
	else if (addr < 0x6000)
	{
		// 5800 - 5FFF, 2K RAM
		data = BIOS[addr - 0x4000];
	}
	else if (addr < 0x6100)
	{
		// 6000 - 60FF, reserved
		data = BIOS[addr - 0x4000];
	}
	else
	{
		// 6100 - 7FFF, RAM
		data = BIOS[addr - 0x4000];
	}

	return data;
}

// 4100 - 7FFF
void MapperBBK::WriteLow(WORD addr, BYTE data)
{
	if (addr < 0x4400)
	{
		// 4100 - 43FF, IO for system
		BIOS[addr - 0x4000] = data;
	}
	else if (addr < 0x5800)
	{
		// 4400 - 57FF, IO for ROM/RAM
		BIOS[addr - 0x4000] = data;
	}
	else if (addr < 0x6000)
	{
		// 5800 - 5FFF, 2K RAM
		BIOS[addr - 0x4000] = data;
	}
	else if (addr < 0x6100)
	{
		// 6000 - 60FF, reserved
		BIOS[addr - 0x4000] = data;
	}
	else
	{
		// 6100 - 7FFF, RAM
		BIOS[addr - 0x4000] = data;
	}
}

// for FDC
void MapperBBK::FdcHardReset(void)
{
	bFdcDmaInt = FALSE;
	nFdcDrvSel = 0;
	nFdcMotor = 0;
	nFdcMainStatus = FDC_MS_RQM;

	nFDCStatus[0] = 0;
	nFDCStatus[1] = 0;
	nFDCStatus[2] = 0;
	nFDCStatus[3] = 0;

	bFdcCycle = 0;
	bFdcPhase = FDC_PH_IDLE;
}

void MapperBBK::FdcSoftReset(void)
{
	nFdcDrvSel = 0;
	nFdcMotor = 0;
	nFdcMainStatus = FDC_MS_RQM;

	nFDCStatus[0] = 0;
	nFDCStatus[1] = 0;
	nFDCStatus[2] = 0;
	nFDCStatus[3] = 0;

	bFdcCycle = 0;
	bFdcPhase = FDC_PH_IDLE;
}

void MapperBBK::FdcNop(MapperBBK* thiz)
{
	thiz->nFDCStatus[0] = FDC_S0_IC1;
	thiz->bFdcResults[0] = thiz->nFDCStatus[0];
}

void MapperBBK::FdcReadTrack(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcSpecify(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcSenseDriveStatus(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcWriteData(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcReadData(MapperBBK* thiz)
{
	BYTE C = thiz->bFdcCommands[2];
	BYTE H = thiz->bFdcCommands[3];
	BYTE R = thiz->bFdcCommands[4];
	BYTE N = thiz->bFdcCommands[5];

	INT LBA;

	LBA = H * 18 + C * 36 + (R - 1);

	thiz->pFdcDataPtr = thiz->DISK + LBA * 512;

	R++;
	if (19 == R)
	{
		R = 1;
		H++;
		if (2 == H)
		{
			C++;
			if (80 == C)
				C = 0;
		}
	}

	thiz->nFDCStatus[0] = 0;

	thiz->bFdcResults[0] = thiz->nFDCStatus[0]; // ST0
	thiz->bFdcResults[1] = thiz->nFDCStatus[1]; // ST1
	thiz->bFdcResults[2] = thiz->nFDCStatus[2]; // ST2
	thiz->bFdcResults[3] = C;
	thiz->bFdcResults[4] = H;
	thiz->bFdcResults[5] = R;
	thiz->bFdcResults[6] = N;
}

void MapperBBK::FdcRecalibrate(MapperBBK* thiz)
{
	BYTE US;

	US = thiz->bFdcCommands[1] & 3;

	if (0 == US)
		thiz->nFDCStatus[0] = FDC_S0_SE;
	else
		thiz->nFDCStatus[0] = FDC_S0_SE | FDC_S0_IC0;
}

void MapperBBK::FdcSenseIntStatus(MapperBBK* thiz)
{
	if (0 == thiz->nFdcDrvSel)	// Drv A Only
		thiz->nFDCStatus[0] = FDC_S0_IC0 | FDC_S0_IC1;
	else
		thiz->nFDCStatus[0] = FDC_S0_SE | FDC_S0_IC0;

	thiz->bFdcResults[0] = thiz->nFDCStatus[0];
	thiz->bFdcResults[1] = thiz->nFdcCylinder;	// PCN
}

void MapperBBK::FdcWriteDeletedData(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcReadID(MapperBBK* thiz)
{
	thiz->nFDCStatus[0] = 0;

	thiz->bFdcResults[0] = thiz->nFDCStatus[0];
}

void MapperBBK::FdcReadDeletedData(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcFormatTrack(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcSeek(MapperBBK* thiz)
{
	// new cylinder number
	BYTE NCN;
	BYTE US;

	US = thiz->bFdcCommands[1] & 3;
	NCN = thiz->bFdcCommands[2];

	thiz->nFdcCylinder = NCN;

	thiz->nFDCStatus[0] = FDC_S0_SE;
}

void MapperBBK::FdcScanEqual(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcScanLowOrEqual(MapperBBK* thiz)
{
	thiz = thiz;
}

void MapperBBK::FdcScanHighOrEqual(MapperBBK* thiz)
{
	thiz = thiz;
}

#endif

/*******************************************************************************
                           E N D  O F  F I L E
*******************************************************************************/
