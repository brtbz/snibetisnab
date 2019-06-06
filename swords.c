/*
Converts png file to custom (hardware) cursor SDL can use.
Only black, white and transparent pixels allowed.
Width and height must both be cleanly divisibly by 8.
Output is C code to stdout

Looks great if you're going for that Windows 3.1 esthetic!

TODO:
actually add input parameters so different files won't require editing and recompiling the code

cc -std=c99 swords.c -lm -o swords
*/

#include <stdio.h>
#include <stdint.h>
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// byteflip function provided by the friendly entities at stackoverflow
uint8_t byteflip(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

int main(int argc, char** argv)
{
	const char *file_path = "swords.png";

	int channels = 4; // so it loads also RGB images as RGBA
	int x,y,n;
	unsigned char *data = stbi_load(file_path, &x, &y, &n, channels);
	printf("%s: %d, %d, %d\n",file_path, x, y, n);

    uint8_t output_data[4*32]; // w * h / 8
    uint8_t output_mask[4*32];

    int j = 0;

    uint8_t pixel_data = 0;
    uint8_t pixel_mask = 0;

    uint8_t packed_pixel_data = 0;
    uint8_t packed_pixel_mask = 0;

    for (int i = 0; i < 32*32; i++)
    {    	;
    	// if ((uint8_t)data[i*4] == 0 && (uint8_t)data[i*4+1] == 0 && (uint8_t)data[i*4+2] == 0 && (uint8_t)data[i*4+3] == 0)
    	if ((uint8_t)data[i*4+3] == 0)
    	{
    		//transparent
    		pixel_data = 0;
    		pixel_mask = 0;
    	}
    	else if ((uint8_t)data[i*4] == 255 && (uint8_t)data[i*4+1] == 255 && (uint8_t)data[i*4+2] == 255 && (uint8_t)data[i*4+3] == 255)
    	{
    		//white
    		pixel_data = 0;
    		pixel_mask = 1;
    	}
    	else if ((uint8_t)data[i*4] == 0 && (uint8_t)data[i*4+1] == 0 && (uint8_t)data[i*4+2] == 0 && (uint8_t)data[i*4+3] == 255)
    	{
    		//black
    		pixel_data = 1;
    		pixel_mask = 1;
    	}
    	else
    	{
    		fprintf(stderr, "ILLEGAL PNG FILE!!!!! white, black and transparent are the only allowed pixels. *dies*\n");
    		exit(1);
    	}

    	uint8_t shifted_pixel_data = pixel_data << (i % 8);
    	uint8_t shifted_pixel_mask = pixel_mask << (i % 8);
    	packed_pixel_data = packed_pixel_data ^ shifted_pixel_data;
    	packed_pixel_mask = packed_pixel_mask ^ shifted_pixel_mask;

    	if (i % 8 == 7)
    	{
    		output_data[j] = packed_pixel_data;
    		output_mask[j] = packed_pixel_mask;
    		packed_pixel_data = 0;
    		packed_pixel_mask = 0;
    		j++;
    	}
    }

    for (int i = 0; i < j; i++)
    {
    	output_mask[i] = byteflip(output_mask[i]);
    	output_data[i] = byteflip(output_data[i]);
    }

    printf("\n");
    printf("uint8_t swords_cursor_data[4*32] = {\n");
    for (int i = 0; i < j; i++)
    {
    	printf("%hhu, ", output_data[i]);
    	if (i % 8 == 7) { printf("\n"); }
    }
    printf("};\n\n");
    printf("uint8_t swords_cursor_mask[4*32] = {\n");
    for (int i = 0; i < j; i++)
    {
    	printf("%hhu, ", output_mask[i]);
    	if (i % 8 == 7) { printf("\n"); }
    }
    printf("};\n");
    printf("SDL_Cursor *swords_cursor = SDL_CreateCursor( &swords_cursor_data[0], &swords_cursor_mask[0], 32, 32, 14, 14 );\n");
	return 0;
}
