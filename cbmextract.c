#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void * checked_malloc( size_t size )
{
    void * temp = malloc( size );
    if( !temp )
    {
        fprintf( stderr, "\nFailed to allocate %zu bytes.\n", size );
        exit( EXIT_FAILURE );
    }
    return temp;
}

int main( int argc, char ** argv )
{
	if( argc != 2 )
	{
		fprintf( stderr, "\nTodo: write better readme.\n" );
		return EXIT_FAILURE;
	}
	FILE * infile = fopen( argv[1], "rb" );
	if( !infile )
	{
		fprintf( stderr, "\nCouldn't open file: %s\n", argv[1] );
		return EXIT_FAILURE;
	}
	uint8_t * buffer = checked_malloc( 0xc );
	if( !fread( buffer, 0xc, 1, infile ) )
	{
		fprintf( stderr, "\nCouldn't read file: %s\n", argv[1] );
		return EXIT_FAILURE;
	}
	printf( "Header:" );
	for( int i = 0; i < 0xc; i++ )
	{
		printf( " %02" PRIx8, buffer[i] );
	}
	uint_fast16_t count = (uint_fast16_t)buffer[0x4] | (uint_fast32_t)buffer[0x5] << 8;
	if( memcmp( buffer, "\377CBM", 4 ) )
	{
		fprintf( stderr, "\nUnrecognized header.\n" );
		return EXIT_FAILURE;
	}
	printf( "\nExtracting %" PRIuFAST16 " files...", count );
	free( buffer );
	buffer = checked_malloc( (size_t)count * 0xc );
	if( !fread( buffer, (size_t)count * 0xc, 1, infile ) )
	{
		fprintf( stderr, "\nCouldn't read file.\n" );
		return EXIT_FAILURE;
	}
	for( uint_fast16_t i = 0; i < count; i++ )
	{
		printf( "\n%" PRIuFAST16 ".JPG: %" PRIuFAST16 "*%" PRIuFAST16 ",", i, (uint_fast16_t)buffer[i * 0xc] | (uint_fast16_t)buffer[i * 0xc + 0x1] << 8, (uint_fast16_t)buffer[i * 0xc + 0x2] | (uint_fast16_t)buffer[i * 0xc + 0x3] << 8 );
		for( int j = 0; j < 0x8; j++ )
		{
			printf( " %02" PRIx8, buffer[i * 0xc + 0x4 + j] );
		}
		if( fseek( infile, (uint_fast32_t)buffer[i * 0xc + 0x4] | (uint_fast32_t)buffer[i * 0xc + 0x5] << 8 | (uint_fast32_t)buffer[i * 0xc + 0x6] << 16 | (uint_fast32_t)buffer[i * 0xc + 0x7] << 24, SEEK_SET ) )
		{
			fprintf( stderr, "\nSeek error.\n" );
			return EXIT_FAILURE;
		}
		uint_fast32_t size = (uint_fast32_t)buffer[i * 0xc + 0x8] | (uint_fast32_t)buffer[i * 0xc + 0x9] << 8 | (uint_fast32_t)buffer[i * 0xc + 0xa] << 16 | (uint_fast32_t)buffer[i * 0xc + 0xb] << 24;
		uint8_t * filebuffer = checked_malloc( size );
		if( !fread( filebuffer, size, 1, infile ) )
		{
			fprintf( stderr, "\nCouldn't read file.\n" );
			return EXIT_FAILURE;
		}
		char filename[10];
		snprintf( filename, 10, "%" PRIuFAST16 ".JPG", i );
		FILE * outfile = fopen( filename, "wb" );
		if( !outfile )
		{
			fprintf( stderr, "\nCouldn't open file: %s\n", filename );
			return EXIT_FAILURE;
		}
		if( !fwrite( filebuffer, size, 1, outfile ) )
		{
			fprintf( stderr, "\nCouldn't write file: %s\n", filename );
			return EXIT_FAILURE;
		}
		fclose( outfile );
		free( filebuffer );
	}
	return EXIT_SUCCESS;
}