#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
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
	uint8_t * buffer = checked_malloc( 0x14 );
	if( !fread( buffer, 0x14, 1, infile ) )
	{
		fprintf( stderr, "\nCouldn't read file: %s\n", argv[1] );
		return EXIT_FAILURE;
	}
	printf( "Header:" );
	for( int i = 0; i < 0x14; i++ )
	{
		printf( " %02" PRIx8, buffer[i] );
	}
	uint_fast32_t count = (uint_fast32_t)buffer[0x4] << 24 | (uint_fast32_t)buffer[0x5] << 16 | (uint_fast32_t)buffer[0x6] << 8 | (uint_fast32_t)buffer[0x7];
	uint_fast32_t size = (uint_fast32_t)buffer[0x8] << 24 | (uint_fast32_t)buffer[0x9] << 16 | (uint_fast32_t)buffer[0xa] << 8 | (uint_fast32_t)buffer[0xb];
	uint_fast32_t offset = (uint_fast32_t)buffer[0xc] << 24 | (uint_fast32_t)buffer[0xd] << 16 | (uint_fast32_t)buffer[0xe] << 8 | (uint_fast32_t)buffer[0xf];
	time_t time = (time_t)buffer[0x10] << 24 | (time_t)buffer[0x11] << 16 | (time_t)buffer[0x12] << 8 | (time_t)buffer[0x13];
	if( buffer[0x0] != 0x00 || buffer[0x1] != 0x14 || count * 0x14 < count || count * 0x14 != size )
	{
		fprintf( stderr, "\nUnrecognized header.\n" );
		return EXIT_FAILURE;
	}
	printf( "\nTimestamp: %sExtracting %" PRIuFAST32 " files...", asctime( gmtime( &time ) ), count );
	free( buffer );
	buffer = checked_malloc( size );
	if( fseek( infile, offset, SEEK_SET ) )
	{
		fprintf( stderr, "\nSeek error.\n" );
		return EXIT_FAILURE;
	}
	if( !fread( buffer, size, 1, infile ) )
	{
		fprintf( stderr, "\nCouldn't read file.\n" );
		return EXIT_FAILURE;
	}
	for( uint_fast32_t i = 0; i < count; i++ )
	{
		if( buffer[i * 0x14 + 3] != 0x00 )
		{
			printf( "\n(error):" );
			for( int j = 0; j < 0x14; j++ )
			{
				printf( " %02" PRIx8, buffer[i * 0x14 + j] );
			}
			fprintf( stderr, "\nUnrecognized entry in table of contents.\n" );
			return EXIT_FAILURE;
		}
		printf( "\n%" PRIuFAST32 ".%s:", i, &buffer[i * 0x14] );
		for( int j = 0; j < 0x14; j++ )
		{
			printf( " %02" PRIx8, buffer[i * 0x14 + j] );
		}
		if( fseek( infile, (uint_fast32_t)buffer[i * 0x14 + 0x10] << 24 | (uint_fast32_t)buffer[i * 0x14 + 0x11] << 16 | (uint_fast32_t)buffer[i * 0x14 + 0x12] << 8 | (uint_fast32_t)buffer[i * 0x14 + 0x13], SEEK_SET ) )
		{
			fprintf( stderr, "\nSeek error.\n" );
			return EXIT_FAILURE;
		}
		size = (uint_fast32_t)buffer[i * 0x14 + 0xc] << 24 | (uint_fast32_t)buffer[i * 0x14 + 0xd] << 16 | (uint_fast32_t)buffer[i * 0x14 + 0xe] << 8 | (uint_fast32_t)buffer[i * 0x14 + 0xf];
		uint8_t * filebuffer = checked_malloc( size );
		if( !fread( filebuffer, size, 1, infile ) )
		{
			fprintf( stderr, "\nCouldn't read file.\n" );
			return EXIT_FAILURE;
		}
		char filename[14];
		snprintf( filename, 14, "%" PRIuFAST32 ".%s", i, &buffer[i * 0x14] );
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