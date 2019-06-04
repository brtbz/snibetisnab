/*
This is supposed to eat six faces of a cubemap in separate ktx files and make one ktx file of them.
Image data is supposed to be compressed (BC7).

step after that is combining several mip levels into one ktx file.
*/

/*
cc -std=c99 ktx_cube.c -o ktx_cube
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>



typedef struct KTX_Header
{
	uint8_t magic_number[12]; // = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	uint32_t endianness; // = 0x01020304;
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
} KTX_Header;

bool ValidateKTXHeader(KTX_Header *header)
{
	const uint8_t ktx_identifier[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

	bool valid = true;
	for (int i = 0; i < 12; i++)
	{
		if ( header->magic_number[i] != ktx_identifier[i] )
		{
			valid = false;
		}
	}

	if (header->bytesOfKeyValueData != 0)
	{
		valid = false;
	}

	return valid;
}

uint8_t *ReadKTX(const char *file_path, KTX_Header *header_out, uint32_t *image_size_out, uint32_t *image_data_offset_out)
{
	FILE *fp = fopen(file_path, "rb");

	if ( fp == NULL)
	{
		fprintf(stderr, "ERROR: Couldn't open file %s\n", file_path);
		exit(1);
	}

	fpos_t begin;
	fgetpos(fp, &begin);
	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	fsetpos(fp, &begin);

	uint8_t* data = malloc(file_size);
	fread(data, file_size, 1, fp);

	fprintf(stderr, "file_size: %d\n", file_size);
	fclose(fp);

	KTX_Header header;
	memcpy(&header, &data[0], sizeof(KTX_Header));

	if (ValidateKTXHeader(&header))
	{
		fprintf(stderr, "valid ktx header for file %s\n", file_path);
		memcpy( header_out, &header, sizeof(KTX_Header));

		*image_size_out = *(uint32_t*)&data[sizeof(KTX_Header)];
		*image_data_offset_out = sizeof(KTX_Header) + sizeof(uint32_t);
		return data;
	}
	else
	{
		fprintf(stderr, "invalid ktx header for file %s\n", file_path);
		return NULL;
	}
}

int main(int argc, char **argv)
{
	KTX_Header ktx_header;
	fprintf(stderr, "size of ktx_header %lu\n", sizeof(ktx_header));

	if (argc < 8)
	{
		fprintf(stderr, "./ktx_cube <+x.ktx> <-x.ktx> <+y.ktx> <-y.ktx> <+z.ktx> <-z.ktx> <cubemap.ktx>\n");
		exit(1);
	}

	KTX_Header header_x_plus;
	uint8_t *data_ptr_x_plus;
	uint32_t image_size_x_plus;
	uint32_t image_data_offset_x_plus;

	KTX_Header header_x_minus;
	uint8_t *data_ptr_x_minus;
	uint32_t image_size_x_minus;
	uint32_t image_data_offset_x_minus;


	KTX_Header header_y_plus;
	uint8_t *data_ptr_y_plus;
	uint32_t image_size_y_plus;
	uint32_t image_data_offset_y_plus;

	KTX_Header header_y_minus;
	uint8_t *data_ptr_y_minus;
	uint32_t image_size_y_minus;
	uint32_t image_data_offset_y_minus;

	KTX_Header header_z_plus;
	uint8_t *data_ptr_z_plus;
	uint32_t image_size_z_plus;
	uint32_t image_data_offset_z_plus;

	KTX_Header header_z_minus;
	uint8_t *data_ptr_z_minus;
	uint32_t image_size_z_minus;
	uint32_t image_data_offset_z_minus;

	KTX_Header cubemap_header;


	data_ptr_x_plus  = ReadKTX(argv[1], &header_x_plus,  &image_size_x_plus,  &image_data_offset_x_plus);
	data_ptr_x_minus = ReadKTX(argv[2], &header_x_minus, &image_size_x_minus, &image_data_offset_x_minus);
	data_ptr_y_plus  = ReadKTX(argv[3], &header_y_plus,  &image_size_y_plus,  &image_data_offset_y_plus);
	data_ptr_y_minus = ReadKTX(argv[4], &header_y_minus, &image_size_y_minus, &image_data_offset_y_minus);
	data_ptr_z_plus  = ReadKTX(argv[5], &header_z_plus,  &image_size_z_plus,  &image_data_offset_z_plus);
	data_ptr_z_minus = ReadKTX(argv[6], &header_z_minus, &image_size_z_minus, &image_data_offset_z_minus);

	uint32_t cubemap_size = sizeof(KTX_Header) + sizeof(uint32_t) + 6 * image_size_x_plus;



	fprintf(stderr, "cubemap size: %u\n", cubemap_size);

	uint8_t *cubemap_data = malloc( cubemap_size );
	memcpy(&cubemap_header, &header_x_plus, sizeof(KTX_Header));
	cubemap_header.numberOfFaces = 6;

	uint32_t image_data_offset = image_data_offset_x_plus;

	uint32_t write_head = 0;
	memcpy(&cubemap_data[write_head], &cubemap_header, sizeof(KTX_Header));
	write_head += sizeof(KTX_Header);

	memcpy(&cubemap_data[write_head], &image_size_x_plus, sizeof(uint32_t));
	write_head += sizeof(uint32_t);
	memcpy(&cubemap_data[write_head], data_ptr_x_plus + image_data_offset, image_size_x_plus);
	write_head += image_size_x_plus;

	memcpy(&cubemap_data[write_head], data_ptr_x_minus + image_data_offset, image_size_x_minus);
	write_head += image_size_x_minus;


	memcpy(&cubemap_data[write_head], data_ptr_y_plus + image_data_offset, image_size_y_plus);
	write_head += image_size_y_plus;


	memcpy(&cubemap_data[write_head], data_ptr_y_minus + image_data_offset, image_size_y_minus);
	write_head += image_size_y_minus;


	memcpy(&cubemap_data[write_head], data_ptr_z_plus + image_data_offset, image_size_z_plus);
	write_head += image_size_z_plus;

	memcpy(&cubemap_data[write_head], data_ptr_z_minus + image_data_offset, image_size_z_minus);
	write_head += image_size_z_minus;

	fprintf(stderr, "wh: %u\n", write_head);

	FILE *new_file = fopen(argv[7], "wb");
	fwrite( &cubemap_data[0], cubemap_size, 1, new_file );
	fclose(new_file);

	free(data_ptr_x_plus);
	free(data_ptr_x_minus);
	free(data_ptr_y_plus);
	free(data_ptr_y_minus);
	free(data_ptr_z_plus);
	free(data_ptr_z_minus);

	free(cubemap_data);

	return 0;
}

/*
1. read six files
2. validate headers. first one against spec, others against first one.
3. write new file (header + 6 x (imageSize + compressed image data))
*/
