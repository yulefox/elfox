#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <string>
#include <elf/base64.h>

namespace elf {

std::string base64_encode(const char *input, int length, bool with_new_line)
{
	BIO *bmem = NULL;
	BIO *b64 = NULL;
	BUF_MEM *bptr = NULL;

	b64 = BIO_new(BIO_f_base64());
	if(!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	(void)BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length + 1);
	memcpy(buff, bptr->data, bptr->length);
	buff[bptr->length] = 0;

	BIO_free_all(b64);
    std::string res(buff);
    free(buff);
    return res;
}

std::string base64_decode(const char *input, int length, bool with_new_line)
{
	BIO *b64 = NULL;
	BIO *bmem = NULL;
	char *buffer = (char *)malloc(length);
	memset(buffer, 0, length);

	b64 = BIO_new(BIO_f_base64());
	if(!with_new_line) {
		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	}
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);

	BIO_read(bmem, buffer, length);

	BIO_free_all(bmem);

    std::string res(buffer);
    free(buffer);
    return res;
}


const char encodeCharacterTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char decodeCharacterTable[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	,-1,62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
	,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
	,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1
};


int b64_encode(unsigned char *input, unsigned input_length, unsigned char *output)
{
	char buff1[3];
	char buff2[4];
	unsigned char i=0, j;
	unsigned input_cnt=0;
	unsigned output_cnt=0;

	while(input_cnt<input_length)
	{
		buff1[i++] = input[input_cnt++];
		if (i==3)
		{
			output[output_cnt++] = encodeCharacterTable[(buff1[0] & 0xfc) >> 2];
			output[output_cnt++] = encodeCharacterTable[((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4)];
			output[output_cnt++] = encodeCharacterTable[((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6)];
			output[output_cnt++] = encodeCharacterTable[buff1[2] & 0x3f];
			i=0;
		}
	}
	if (i)
	{
		for(j=i;j<3;j++)
		{
			buff1[j] = '\0';
		}
		buff2[0] = (buff1[0] & 0xfc) >> 2;
		buff2[1] = ((buff1[0] & 0x03) << 4) + ((buff1[1] & 0xf0) >> 4);
		buff2[2] = ((buff1[1] & 0x0f) << 2) + ((buff1[2] & 0xc0) >> 6);
		buff2[3] = buff1[2] & 0x3f;
		for (j=0;j<(i+1);j++)
		{
			output[output_cnt++] = encodeCharacterTable[buff2[j]];
		}
		while(i++<3)
		{
			output[output_cnt++] = '=';
		}
	}
	output[output_cnt] = 0;
    return output_cnt;
}

int b64_decode(const char *input2, unsigned input_length, unsigned char *output)
{
	char buff1[4];
	char buff2[4];
	unsigned char i=0, j;
	unsigned input_cnt=0;
	unsigned output_cnt=0;
    const unsigned char *input =  (const unsigned char*)(input2);

	while(input_cnt<input_length)
	{
		buff2[i] = input[input_cnt++];
		if (buff2[i] == '=')
		{
			break;
		}
        if (buff2[i] == '\n') {
            continue;
        }
		if (++i==4)
		{
			for (i=0;i!=4;i++)
			{
				buff2[i] = decodeCharacterTable[buff2[i]];
			}
			output[output_cnt++] = (char)((buff2[0] << 2) + ((buff2[1] & 0x30) >> 4));
			output[output_cnt++] = (char)(((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2));
			output[output_cnt++] = (char)(((buff2[2] & 0x3) << 6) + buff2[3]);
			i=0;
		}
	}
	if (i)
	{
		for (j=i;j<4;j++)
		{
			buff2[j] = '\0';
		}
		for (j=0;j<4;j++)
		{
			buff2[j] = decodeCharacterTable[buff2[j]];
		}
		buff1[0] = (buff2[0] << 2) + ((buff2[1] & 0x30) >> 4);
		buff1[1] = ((buff2[1] & 0xf) << 4) + ((buff2[2] & 0x3c) >> 2);
		buff1[2] = ((buff2[2] & 0x3) << 6) + buff2[3];
		for (j=0;j<(i-1); j++)
		{
			output[output_cnt++] = (char)buff1[j];
		}
	}
	output[output_cnt] = 0;
    return output_cnt;
}


} // namespace elf
