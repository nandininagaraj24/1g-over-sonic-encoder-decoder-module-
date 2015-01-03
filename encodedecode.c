#include "stdlib.h"
#include <time.h>
#include <sys/times.h>
#include "codec_8b10b.h"
#include <inttypes.h>

/* Negative look up table to encode 3-4 bit*/
static const uint8_t enc_34_neg[] = {
    0b1011,
    0b1001,
    0b0101,
    0b1100,
    0b1101,
    0b1010,
    0b0110,
    0b1110 // 0b0111 - alternate
};

/* Positive look up table to encode 3-4 bit*/
static const uint8_t enc_34_pos[] = {
    0b0100,
    0b1001,
    0b0101,
    0b0011,
    0b0010,
    0b1010,
    0b0110,
    0b0001, // 0b1000 - alternate
};

/* Positive look up table to encode 3-4 bit*/
static const uint8_t enc_56_pos[] = {
    0b011000,
    0b100010,
    0b010010,
    0b110001,
    0b001010,
    0b101001,
    0b011001,
    0b000111,
    0b000110,
    0b100101,
    0b010101,
    0b110100,
    0b001101,
    0b101100,
    0b011100,
    0b101000,
    0b100100,
    0b100011,
    0b010011,
    0b110010,
    0b001011,
    0b101010,
    0b011010,
    0b000101,
    0b001100,
    0b100110,
    0b010110,
    0b001001,
    0b001110,
    0b010001,
    0b100001,
    0b010100,
};

/* Negative look up table to encode 3-4 bit*/
static const uint8_t enc_56_neg[] = {
    0b100111,
    0b011101,
    0b101101,
    0b110001,
    0b110101,
    0b101001,
    0b011001,
    0b111000,
    0b111001,
    0b100101,
    0b010101,
    0b110100,
    0b001101,
    0b101100,
    0b011100,
    0b010111,
    0b011011,
    0b100011,
    0b010011,
    0b110010,
    0b001011,
    0b101010,
    0b011010,
    0b111010,
    0b110011,
    0b100110,
    0b010110,
    0b110110,
    0b001110,
    0b101110,
    0b011110,
    0b101011
};

/* running disparity */
static int RD = +1;

/*
 * decoding stuff
 */
#define INVALID_VALUE         0xFF
static uint8_t dec_65[64];
static uint8_t dec_43[16];

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
/**
 *
 */
/* Finding number of 1s minus number of 0s*/
static int block_disparity(uint32_t data, size_t bits){
/*data = data - ((data >> 1) & 0x55555555);
     data = (data & 0x33333333) + ((data >> 2) & 0x33333333);
     data= (((data + (data >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
return data-(bits-data);*/

return __builtin_popcount(data)-(bits-__builtin_popcount(data));


}

/**
 *
 */
static void decode_tab_helper(const uint8_t *enc_array,
                      size_t enc_array_len, uint8_t *dec_array){
  size_t idx;
  uint8_t val;
  for (val=enc_array_len;val--;){
    idx = enc_array[val];
    osalDbgCheck((dec_array[idx] == val) || (dec_array[idx] == INVALID_VALUE));
    dec_array[idx] = val;
  }

}

/**
 *
 */
static void fill_decode_tables(void){
  size_t i = 0;
  for (i=sizeof(dec_65); i--;)
    dec_65[i] = INVALID_VALUE;
  decode_tab_helper(enc_56_neg, sizeof(enc_56_neg), dec_65);
  decode_tab_helper(enc_56_pos, sizeof(enc_56_pos), dec_65);

  for (i=sizeof(dec_43); i--;)
    dec_43[i] = INVALID_VALUE;
  decode_tab_helper(enc_34_neg, sizeof(enc_34_neg), dec_43);
  decode_tab_helper(enc_34_pos, sizeof(enc_34_pos), dec_43);
}


/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
void codec_8b10b_init(void){
  fill_decode_tables();
}

/**
 * The encoding function
 * 
 */
uint16_t encode_8b10b(uint8_t data){
  uint16_t ret = 0;
  uint16_t tmp = 0;
  uint8_t EDCBA = data & 0b11111;
  uint8_t HGF   = (data >> 5) & 0b111;
  if (RD <= 0)
{
    tmp = enc_56_neg[EDCBA];
}	
  else
{
    tmp = enc_56_pos[EDCBA];
}
  RD += block_disparity(tmp, 6);
  ret = tmp << 4;

  if (RD <= 0)
{
    tmp = enc_34_neg[HGF];
}
  else
{
    tmp = enc_34_pos[HGF];
}
  RD += block_disparity(tmp, 4);
  ret |= tmp;

  return ret;
}

/**
 *
 */
uint8_t decode_8b10b(uint16_t code){
  osalDbgCheck(code <= 1023);

  uint8_t EDCBA;
  uint8_t HGF;
  size_t idx;

  idx = code & 0b1111;
  HGF = dec_43[idx];

  idx = code >> 4;
  EDCBA = dec_65[idx];

  if ((INVALID_VALUE == HGF) || (INVALID_VALUE == EDCBA))
    return DECODE_FAILED;
  else
    return HGF << 5 | EDCBA;
}


int main(){
	time_t start, stop;
//	time_t begin,end;
	clock_t ticks;
//	double time_spent;
	codec_8b10b_init();
	time(&start);
//	begin=clock();
	uint8_t enc = 179;
	scanf("%hu", &enc);
	ticks = clock();
	uint16_t dec = encode_8b10b(enc);
	time(&stop);
//	end=clock();
//	time_spent=(double)(end - begin) / CLOCKS_PER_SEC;
	printf("%d -> %d\n", enc, dec);
	printf("Used %f seconds of CPU time. \n", (double)ticks/CLOCKS_PER_SEC);
	printf("Finished in about %f seconds. \n", difftime(stop, start));
//	printf("Used %f seconds of CPU timetime spent:. \n",time_spent);
	uint16_t odec = 204;
	scanf("%hu", &odec);
	uint8_t oenc = decode_8b10b(odec);
	printf("%d -> %d\n", odec, oenc);

	return 0;
}
