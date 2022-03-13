/*!\file OW_crc.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire CRC
** \note The 1-Wire CRC scheme is described in Maxim Application Note 27:
** 		"Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_crc.h"
/****************************************************************/


#if ONEWIRE_CRC8_TABLE
static const uint8_t dscrc_table[] = {
	  0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65,
	157, 195,  33, 127, 252, 162,  64,  30,  95,   1, 227, 189,  62,  96, 130, 220,
	 35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,  60,  98,
	190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,
	 70,  24, 250, 164,  39, 121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7,
	219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251, 120,  38, 196, 154,
	101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36,
	248, 166,  68,  26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185,
	140, 210,  48, 110, 237, 179,  81,  15,  78,  16, 242, 172,  47, 113, 147, 205,
	 17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80,
	175, 241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,
	 50, 108, 142, 208,  83,  13, 239, 177, 240, 174,  76,  18, 145, 207,  45, 115,
	202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213, 139,
	 87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22,
	233, 183,  85,  11, 136, 214,  52, 106,  43, 117, 151, 201,  74,  20, 246, 168,
	116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215, 137, 107,  53 };	/*!< This table comes from Dallas sample code where it is freely reusable,
																						**		though Copyright (C) 2000 Dallas Semiconductor Corporation **/


void NONNULL__ OWCompute_DallasCRC8(uint8_t * const pCRC8, const uint8_t * const pData, const uint16_t nb)
{
	for (int i = 0 ; i < nb ; i++)
	{
		const uint8_t data = pData[i];
		*pCRC8 = dscrc_table[(*pCRC8 ^ data)];
	}
}

#else

void NONNULL__ OWCompute_DallasCRC8(uint8_t * const pCRC8, const uint8_t * const pData, const uint16_t nb)
{
	for (int i = 0 ; i < nb ; i++)
	{
		uint8_t data = pData[i];

		for (int j = 8 ; j ; j--)
		{
			const uint8_t mix = (*pCRC8 ^ data) & 0x01;

			*pCRC8 >>= 1;

			if (mix)	{ *pCRC8 ^= 0x8C; }

			data >>= 1;
		}
	}
}

#endif


FctERR NONNULL__ OWCheck_DallasCRC8(const uint8_t * const pData, const uint16_t nb, const uint8_t crc8)
{
	uint8_t crc = 0;
	OWCompute_DallasCRC8(&crc, pData, nb);

	return (crc == crc8) ? ERROR_OK : ERROR_CRC;
}


void NONNULL__ OWCompute_DallasCRC16(uint16_t * const pCRC16, const uint8_t * const pData, const uint16_t nb)
{
	for (int i = 0 ; i < nb ; i++)
	{
		uint8_t data = pData[i];

		for (int j = 8 ; j ; j--)
		{
			const uint8_t mix = ((*pCRC16 & 0xFF) ^ data) & 0x01;

			*pCRC16 >>= 1;

			if (mix) { *pCRC16 ^= 0xA001; }

			data >>= 1;
		}
	}
}

FctERR NONNULL__ OWCheck_DallasCRC16(const uint8_t * const pData, const uint16_t nb, const uint16_t icrc16)
{
	uint16_t crc = 0;
	OWCompute_DallasCRC16(&crc, pData, nb);
	crc = ~crc;

	return (crc == icrc16) ? ERROR_OK : ERROR_CRC;
}
