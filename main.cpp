/*
 * FlashAir Developers Summit 2018
 * いぶし銀カメラ for GR-LYCHEE + OpenCV
 * Ibushigin Camera for GR-LYCHEE + OpenCV
 * Mbed CLI Compiler
 * FlashAir Developers : https://www.flashair-developers.com/ja/
 * GR-LYCHEE : https://os.mbed.com/platforms/Renesas-GR-LYCHEE/
 * AS-289R2 Thermal Printer Shield : http://www.nada.co.jp/as289r2/
 * Public Domain
 */

#include "mbed.h"
#include "EasyAttach_CameraAndLCD.h"
#include "SdUsbConnect.h"
#include "opencv.hpp"
#include "camera_if.hpp"
#include "DisplayApp.h"
#include "AS289R2.h"

using namespace cv;

AS289R2 tp(D1, 38400);	// 38400bps
DigitalIn SW(D13);
DigitalOut led1( LED1 );
DigitalOut led2( LED2 );
DigitalOut led3( LED3 );
DigitalOut led4( LED4 );

/* Application variables */
Mat src, dst;
char file_name[32];
int file_name_index = 0;

// To monitor realtime on PC, you need DisplayApp on following site.
// Connect USB0(not for mbed interface) to your PC
// https://os.mbed.com/users/dkato/code/DisplayApp/

/* For viewing image on PC */
static DisplayApp display_app;

// Image Print
// BMP(Depth=8bit) -> AS-289R2
void BmpToAS289R2( int filenumber )
{
	char bmpFilename[32];
	sprintf( bmpFilename, "/storage/%d_color2.bmp", filenumber );
	FILE * fp = fopen( bmpFilename, "r" );
	if ( fp != NULL )
	{
		// Get bmpOffset
		fseek( fp, 10, SEEK_SET );
		int bmpOffset = ( int )( fgetc( fp ) );
		bmpOffset += ( int )( fgetc( fp ) * 256 );
		// Get bmpHeight
		fseek( fp, 22, SEEK_SET );
		int bmpHeight = ( int )( fgetc( fp ) );
		bmpHeight += ( int )( fgetc( fp ) * 256 );
		// AS-289R2 CMD
		tp.printf( "\x1C\x2A\x65" );
		tp.putc( ( uint8_t )( bmpHeight / 256 ) );
		tp.putc( ( uint8_t )( bmpHeight % 256 ) );
		// Put Bmp-data
		for ( int iy = 1; iy <= bmpHeight; iy ++ )
		{
			int LeftPoint = bmpHeight * 48 * 8 + bmpOffset - ( iy * 48 * 8 );
			fseek( fp, LeftPoint, SEEK_SET );
			for ( int ix = 0; ix < 48; ix ++ )
			{
				uint8_t pixel8 = 0;
				for ( int ib = 0; ib < 8; ib ++ )
				{
					uint8_t pixel = ( uint8_t )( fgetc( fp ) ^ 0xFF );
					pixel8 <<= 1;
					if (pixel && 0xFF)
					{
						pixel8 |= 1;
					}
				}
				tp.putc( pixel8 );
				wait_us( 50 );
			}
		}
	}
	fclose( fp );
}


/**
 * Dithering Algorithm
 */

uint8_t saturated_add( uint8_t val1, int8_t val2 )
{
	int16_t val1_int = val1;
	int16_t val2_int = val2;
	int16_t tmp = val1_int + val2_int;

	if( tmp > 255 )
	{
		return 255;
	}
	else if( tmp < 0 )
	{
		return 0;
	}
	else
	{
		return tmp;
	}
}

void Dithering()
{
	int dstWidth = dst.cols;
	int dstHeight = dst.rows;
	int err;
	int8_t a, b, c, d;

	for( int i = 0; i < dstHeight; i++ )
	{
		for( int j = 0; j < dstWidth; j++ )
		{
			if( dst.at<uint8_t>( i, j ) > 127 )
			{
				err = dst.at<uint8_t>( i, j ) - 255;
				dst.at<uint8_t>( i, j ) = 255;
			}
			else
			{
				err = dst.at<uint8_t>( i, j ) - 0;
				dst.at<uint8_t>( i, j ) = 0;
			}
			a = ( err * 7 ) / 16;
			b = ( err * 1 ) / 16;
			c = ( err * 5 ) / 16;
			d = ( err * 3 ) / 16;
			if( ( i != ( dstHeight - 1 ) ) && ( j != 0 ) && ( j != ( dstWidth - 1 ) ) )
			{
				dst.at<uint8_t>( i + 0, j + 1 ) = saturated_add( dst.at<uint8_t>( i + 0, j + 1 ), a );
				dst.at<uint8_t>( i + 1, j + 1 ) = saturated_add( dst.at<uint8_t>( i + 1, j + 1 ), b );
				dst.at<uint8_t>( i + 1, j + 0 ) = saturated_add( dst.at<uint8_t>( i + 1, j + 0 ), c );
				dst.at<uint8_t>( i + 1, j - 1 ) = saturated_add( dst.at<uint8_t>( i + 1, j - 1 ), d );
			}
		}
	}
}


//
// Main
//

int main()
{
	// Camera
	printf( "Initializing the Camera...\r\n" );
	camera_start();
	printf( "done\n" );
	led4 = 1;

	// SD & USB
	SdUsbConnect storage( "storage" );
	printf( "Finding a storage..." );
	// wait for the storage device to be connected
	storage.wait_connect();
	printf( "done\n" );
	led3 = 1;

	while ( 1 )
	{
		// Retrieve a video frame (grayscale)
		create_gray( src );
		if ( src.empty() )
		{
			printf( "ERR: There is no input frame, retry to capture\r\n" );
			continue;
		}

		// SW on ?
		if (SW == 0) {
			dst = src.clone();	// src to dst

			led2 = 1;
			file_name_index++;
			resize( dst, dst, cv::Size(), 384.0 / dst.cols, 384.0 / dst.cols ); // OpenCV - resize
			sprintf( file_name, "/storage/%d_gray.bmp", file_name_index );
			imwrite( file_name, dst );	// OpenCV - imwrite
			Dithering();				// Dithering
			flip(dst, dst, 0);			// OpenCV - flip
			sprintf( file_name, "/storage/%d_color2.bmp", file_name_index );
			imwrite( file_name, dst );	// OpenCV - imwrite
			led2 = 0;

			led1 = 1;
			BmpToAS289R2( file_name_index );	// Image Print
			tp.printf( "\x1B\x49\x01" );
			tp.setANKFont( 0 );
			tp.putLineFeed( 1 );
			tp.printf( "Produced by NADA ELECTRONICS\r" );
			tp.putPaperFeed(8);
			tp.printf( "AS-289R2 Thermal Printer Shield\r" );
			tp.printf( "FlashAir W-04\r" );
			tp.printf( "GR-LYCHEE + Camera + OpenCV\r" );
			tp.putPaperFeed( 8 );
			tp.setANKFont( 1 );
			tp.putPaperFeed( 15 );
			tp.printf( "        いぶし銀カメラ\r" );
			tp.putPaperFeed(15);
			tp.printf( "FlashAir Developer Summit2018\r\r" );
			tp.putLineFeed( 8 );
			led1 = 0;
		}

		// PC MONITOR
		size_t jpeg_size = create_jpeg();
		display_app.SendJpeg( get_jpeg_adr(), jpeg_size );
	}
}

