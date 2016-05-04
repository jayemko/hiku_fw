#include <stdio.h>
#include <stdlib.h>
#include <lodepng.h>
#include <zbar.h>
#include <stdint.h>

zbar_image_scanner_t *scanner = NULL;
uint8_t teststring[1140] = {0xd9,0xdc,0xdc,0xd9,0xdb,0xdd,0xda,0xd8,0xdd,0xda,0xd8,0xdc,0xd9,0xd6,0xda,0xda,0xd8,0xdb,0xd7,0xd6,0xda,0xd9,0xd8,0xd8,0xd8,0xd9,0xdb,0xd6,0xd7,0xd9,0xd8,0xdb,0xdb,0xd4,0xd7,0xd8,0xd7,0xd9,0xd9,0xd5,0xda,0xd7,0xd8,0xd5,0xd6,0xd8,0xdc,0xd6,0xd6,0xd6,0xd6,0xd9,0xdb,0xd5,0xd5,0xd7,0xd7,0xda,0xd9,0xd5,0xd4,0xd6,0xda,0xdc,0xd9,0xd6,0xd7,0xd5,0xd8,0xda,0xda,0xd8,0xd7,0xd5,0xd7,0xd8,0xda,0xdb,0xd6,0xd7,0xd9,0xd6,0xd9,0xd9,0xd5,0xd9,0xdb,0xd6,0xd7,0xd3,0xd5,0xd8,0xda,0xd4,0xd6,0xd6,0xd4,0xd5,0xd5,0xd4,0xd2,0xd7,0xd6,0xd6,0xd5,0xd5,0xd2,0xd3,0xa6,0xa2,0xa1,0x9a,0x95,0x93,0x46,0x46,0x47,0x48,0x41,0x43,0xa2,0xaa,0xaf,0xb3,0xab,0xac,0xcf,0xcc,0xcb,0xcd,0xcb,0xcc,0x6a,0x64,0x67,0x68,0x62,0x5f,0x87,0x8d,0x97,0x94,0x91,0x92,0xd4,0xd7,0xd8,0xd7,0xd7,0xd6,0xcd,0xd0,0xd1,0xd1,0xd0,0xd1,0xd2,0xd1,0xd1,0xcf,0xcf,0xcf,0x83,0x7f,0x7e,0x76,0x72,0x6f,0x5e,0x5e,0x60,0x62,0x6c,0x6e,0xc8,0xcb,0xca,0xcb,0xd1,0xcf,0xcf,0xd4,0xd3,0xd1,0xd0,0xd1,0xcf,0xcf,0xd3,0xd2,0xd3,0xd2,0xd1,0xce,0xd0,0xd2,0xd1,0xd5,0xcc,0xc5,0xc9,0xc8,0xc5,0xc8,0x70,0x67,0x6b,0x65,0x61,0x62,0x81,0x85,0x87,0x90,0x91,0x91,0xd2,0xd2,0xd2,0xd1,0xd2,0xd1,0x8e,0x88,0x81,0x7c,0x79,0x73,0x4d,0x47,0x49,0x4a,0x49,0x51,0xb4,0xb8,0xb9,0xb8,0xb8,0xc2,0xd0,0xd0,0xce,0xc7,0xcb,0xd7,0xcd,0xcb,0xcd,0xc8,0xcd,0xd6,0x9c,0x97,0x94,0x91,0x91,0x8c,0x32,0x2e,0x2e,0x30,0x2a,0x29,0x23,0x22,0x24,0x27,0x25,0x26,0x27,0x25,0x29,0x28,0x25,0x24,0x20,0x22,0x26,0x26,0x20,0x20,0x6c,0x6b,0x70,0x76,0x7e,0x7c,0xca,0xc5,0xcc,0xca,0xcb,0xcb,0x93,0x8e,0x93,0x8a,0x85,0x82,0x43,0x43,0x48,0x48,0x4b,0x4e,0xa6,0xac,0xaf,0xb3,0xb6,0xb8,0xce,0xcc,0xcc,0xc9,0xca,0xcc,0xcc,0xc9,0xca,0xc7,0xca,0xce,0x9d,0x9e,0x9d,0x95,0x91,0x93,0x31,0x33,0x37,0x2f,0x2f,0x30,0x20,0x27,0x23,0x23,0x27,0x23,0x27,0x28,0x24,0x28,0x24,0x22,0x1f,0x22,0x1f,0x20,0x1e,0x20,0x50,0x56,0x5a,0x5e,0x62,0x6c,0xbe,0xc1,0xc3,0xc6,0xc8,0xc8,0xc9,0xcd,0xcc,0xc9,0xca,0xc8,0xcb,0xce,0xcc,0xcc,0xcf,0xcd,0xa6,0xa0,0x9d,0x9c,0x95,0x92,0x57,0x56,0x56,0x58,0x58,0x5b,0xb1,0xb3,0xb7,0xb8,0xbb,0xc1,0xd0,0xcd,0xcc,0xcd,0xcc,0xce,0xcd,0xcb,0xcc,0xcc,0xc9,0xcd,0xce,0xcc,0xcd,0xc9,0xc9,0xcd,0xcf,0xcf,0xcf,0xcc,0xcd,0xcd,0xac,0xa7,0xa2,0x9f,0x9f,0x94,0x53,0x55,0x54,0x56,0x5a,0x59,0xa7,0xae,0xb1,0xb3,0xb1,0xbc,0xb9,0xb1,0xb0,0xab,0xa6,0xa4,0x4a,0x47,0x4b,0x47,0x47,0x48,0x7b,0x84,0x86,0x8a,0x92,0x9c,0xc9,0xce,0xc9,0xc9,0xc9,0xcc,0xc1,0xc6,0xc6,0xc6,0xc5,0xc9,0xc1,0xc1,0xbe,0xbb,0xb8,0xb6,0x6d,0x67,0x5d,0x58,0x57,0x4d,0x1d,0x1c,0x1c,0x1c,0x1d,0x1d,0x1c,0x1f,0x24,0x21,0x1f,0x22,0x1a,0x1a,0x1b,0x1b,0x1b,0x1b,0x26,0x26,0x2a,0x2d,0x2f,0x37,0x90,0x94,0x9a,0x9e,0x9e,0xa9,0xbe,0xbd,0xbb,0xbb,0xb9,0xb3,0x63,0x5a,0x57,0x52,0x4f,0x4a,0x5f,0x60,0x69,0x73,0x78,0x81,0xc2,0xc2,0xc6,0xc7,0xc7,0xc9,0xc4,0xc3,0xc2,0xbf,0xc4,0xc5,0xbe,0xbe,0xbf,0xbb,0xbb,0xb9,0x74,0x6e,0x6d,0x62,0x5d,0x5c,0x1f,0x21,0x20,0x1c,0x1f,0x20,0x1e,0x1d,0x1e,0x1e,0x20,0x21,0x1d,0x1d,0x1d,0x1e,0x1d,0x1b,0x1c,0x20,0x22,0x25,0x23,0x29,0x78,0x7f,0x89,0x8c,0x89,0x98,0xbb,0xbc,0xbe,0xbe,0xbb,0xbb,0x6b,0x6a,0x63,0x61,0x5a,0x4f,0x3e,0x3f,0x46,0x4d,0x54,0x59,0xaf,0xb3,0xb8,0xbc,0xc1,0xc1,0xb3,0xae,0xa6,0xa1,0x9e,0x96,0x53,0x4d,0x49,0x4c,0x51,0x4e,0x8b,0x91,0x9d,0xa6,0xaf,0xaf,0xc0,0xc0,0xbe,0xbc,0xba,0xb8,0x6f,0x65,0x5a,0x56,0x52,0x4d,0x44,0x48,0x4c,0x53,0x5c,0x64,0xaf,0xb3,0xb6,0xba,0xbe,0xbb,0x9a,0x98,0x8e,0x89,0x84,0x75,0x31,0x31,0x2f,0x2c,0x2a,0x26,0x18,0x1c,0x1b,0x18,0x18,0x1c,0x2d,0x33,0x37,0x40,0x47,0x4f,0x9d,0xa6,0xa9,0xaf,0xb6,0xba,0xc2,0xc3,0xc3,0xc4,0xc5,0xc7,0xc0,0xc4,0xc6,0xc4,0xc2,0xc5,0xc3,0xc6,0xc5,0xc3,0xc2,0xc3,0xc4,0xc4,0xc5,0xc2,0xc1,0xbd,0x78,0x74,0x6f,0x63,0x5d,0x57,0x3d,0x3e,0x44,0x4f,0x52,0x59,0xab,0xad,0xb1,0xbc,0xbb,0xbd,0xaa,0xa1,0x97,0x97,0x91,0x87,0x43,0x39,0x36,0x34,0x31,0x2e,0x1d,0x1c,0x1b,0x19,0x17,0x1b,0x2b,0x30,0x39,0x41,0x46,0x4c,0x9e,0xa1,0xac,0xb3,0xb2,0xb7,0xc5,0xc2,0xc3,0xc7,0xc3,0xc5,0xc5,0xc5,0xc5,0xc6,0xc4,0xc6,0xc6,0xc6,0xc6,0xc6,0xc1,0xc4,0xcb,0xca,0xc9,0xc8,0xbe,0xc2,0x85,0x7e,0x78,0x72,0x65,0x5f,0x55,0x56,0x62,0x67,0x70,0x7c,0xba,0xbe,0xc4,0xc1,0xc3,0xc9,0xbf,0xc6,0xc6,0xc4,0xc4,0xc4,0xc0,0xc6,0xc8,0xc4,0xc3,0xc4,0xc2,0xc6,0xc7,0xc6,0xc5,0xc6,0xc3,0xc4,0xbf,0xba,0xb8,0xb5,0x73,0x6a,0x65,0x5b,0x54,0x4e,0x28,0x23,0x22,0x21,0x21,0x24,0x1d,0x1f,0x1f,0x24,0x2a,0x32,0x74,0x81,0x8a,0x94,0x9c,0xa2,0xc3,0xc4,0xc1,0xbd,0xb7,0xb8,0x75,0x6b,0x5f,0x5f,0x55,0x4f,0x5f,0x62,0x6b,0x7e,0x81,0x89,0xc1,0xc0,0xc2,0xc8,0xc3,0xc9,0xc0,0xbf,0xc2,0xbf,0xc0,0xc5,0xc1,0xbf,0xbe,0xb9,0xb5,0xb5,0x72,0x68,0x5e,0x52,0x4d,0x49,0x23,0x23,0x24,0x24,0x25,0x22,0x1d,0x23,0x25,0x2d,0x2f,0x2e,0x7f,0x8a,0x8d,0x9b,0xa2,0xa7,0xc4,0xc0,0xc4,0xc6,0xc3,0xc2,0xc1,0xc0,0xc3,0xc2,0xc2,0xc8,0xc2,0xbd,0xb7,0xad,0xa3,0x9f,0x60,0x57,0x50,0x43,0x3d,0x36,0x24,0x21,0x22,0x1f,0x1f,0x1e,0x2f,0x33,0x3f,0x47,0x50,0x5b,0x9d,0xa8,0xb4,0xb7,0xbb,0xbf,0xc3,0xc1,0xc6,0xc3,0xc3,0xc4,0xc3,0xbf,0xc3,0xc2,0xc1,0xc6,0xc5,0xc3,0xc6,0xc2,0xc2,0xca,0xc7,0xc1,0xbe,0xb1,0xa8,0xa4,0x5f,0x56,0x49,0x45,0x43,0x39,0x6b,0x72,0x72,0x82,0x93,0x99,0xc4,0xc9,0xc3,0xbc,0xba,0xba,0x72,0x6c,0x5f,0x5a,0x4f,0x4a,0x51,0x59,0x5f,0x71,0x7b,0x82,0xc3,0xc6,0xc1,0xbf,0xc9,0xc0,0x9e,0x90,0x7a,0x72,0x6f,0x63,0x3b,0x33,0x29,0x2b,0x29,0x2b,0x28,0x25,0x24,0x24,0x25,0x2b,0x5f,0x6d,0x7c,0x8a,0x96,0x9c,0xc4,0xc6,0xc6,0xc7,0xc8,0xc7,0xc6,0xc6,0xc3,0xc2,0xc2,0xc1,0xc5,0xc6,0xc2,0xc2,0xc4,0xc4,0xcd,0xcb,0xc8,0xc9,0xc3,0xc2,0x98,0x86,0x74,0x72,0x61,0x5a,0x44,0x44,0x41,0x4c,0x55,0x6b,0xab,0xb7,0xbc,0xc0,0xc5,0xcd,0xae,0xac,0xa6,0x92,0x83,0x7f,0x48,0x4c,0x51,0x56,0x5c,0x61,0xa2,0xab,0xb4,0xbe,0xc1,0xc2,0xcb,0xc9,0xc5,0xc4,0xc5,0xc6,0xc7,0xc7,0xc0,0xc2,0xc7,0xc8,0xc8,0xca,0xc8,0xc5,0xc8,0xc6,0xc9,0xcb,0xca,0xc6,0xc9,0xc7,0xcc,0xcc,0xc6,0xc9,0xc9,0xc9,0xcb,0xcd,0xc5,0xca,0xc7,0xc9,0xca,0xcc,0xc7,0xcb,0xc6,0xc8,0xc7,0xca,0xcd,0xc9,0xc6,0xc9,0xc4,0xcc,0xcd,0xc6,0xc6,0xce,};
//unsigned char teststring[] = {228,255,41,0,80,12,74,119,135,161,208,185,0,0,0,0,236,255,41,0,31,158,72,119,255,255,255,255,253,214,74,119,0,0,0,0,0,0,0,0,160,18,64,0,0,224,253,127,0,0,0,0,65,99,116,120,32,0,0,0,1,0,0,0,92,51,0,0,220,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,20,0,0,0,1,0,0,0,7,0,0,0,52,0,0,0,124,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,78,239,38,26,152,2,0,0,68,0,0,0,224,2,0,0,100,2,0,0,0,0,0,0,186,113,50,243,68,5,0,0,74,0,0,0,144,5,0,0,78,3,0,0,0,0,0,0,91,73,89,45,224,8,0,0,50,0,0,0,20,9,0,0,6,3,0,0,0,0,0,0,205,234,206,50,28,12,0,0,66,0,0,0,96,12,0,0,60,3,0,0,0,0,0,0,200,95,80,56,156,15,0,0,94,0,0,0,252,15,0,0,110,3,0,0,0,0,0,0,68,5,40,177,108,19,0,0,86,0,0,0,196,19,0,0,158,3,0,0,16,0,0,0,9,0,0,0,236,0,0,0,2,0,0,0,1,0,0,0,124,1,0,0,32,22,0,0,1,0,0,0,2,0,0,0,156,23,0,0,160,7,0,0,1,0,0,0,3,0,0,0,60,31,0,0,140,14,0,0,1,0,0,0,4,0,0,0,200,45,0,0,20,3,0,0,2,0,0,0,5,0,0,0,220,48,0,0,152,0,0,0,2,0,0,0,6,0,0,0,116,49,0,0,204,0,0,0,2,0,0,0,7,0,0,0,64,50,0,0,240,0,0,0,1,0,0,0,9,0,0,0,48,51,0,0,40,0,0,0,2,0,0,0,11,0,0,0,88,51,0,0,4,0,0,0,1,0,0,0,83,115,72,100,44,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,6,0,0,0,140,0,0,0,1,0,0,0,232,21,0,0,44,0,0,0,94,0,0,0,94,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,36,0,0,0,56,0,0,0,0,0,0,0,67,0,58,0,92,0,87,0,105,0,110,0,100,0,111,0,119,0,115,0,92,0,87,0,105,0,110,0,83,0,120,0,115,0,92,0,0,0,0,0,78,239,38,26,28,1,0,0,68,0,0,0,100,1,0,0,100,2,0,0,1,0,0,0,186,113,50,243,200,3,0,0,74,0,0,0,20,4,0,0,78,3,0,0,2,0,0,0,91,73,89,45,100,7,0,0,50,0,0,0,152,7,0,0,6,3,0,0,3,0,0,0,205,234,206,50,160,10,0,0,66,0,0,0,228,10,0,0,60,3,0,0,4,0,0,0,200,95,80,56,32,14,0,0,94,0,0,0,128,14,0,0,110,3,0,0,5,0,0,0,68,5,40,177,240,17,0,0,86,0,0,0,72,18,0,0,158,3,0,0,6,0,0,0,77,0,105,0,99,0,114,0,111,0,115,0,111,0,102,0,116,0,46,0,87,0,105,0,110,0,100,0,111,0,119,0,115,0,46,0,83,0,121,0,115,0,116,0,101,0,109,0,67,0,111,0,109,0,112,0,97,0,116,0,105,0,98,0,108,0,101,0,0,0,0,0,108,0,0,0,1,0,0,0,14,1,0,0,208,1,0,0,2,0,0,0,44,0,0,0,222,2,0,0,163,112,83,58,255,186,208,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,186,0,0,0,12,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,77,0,105,0,99,0,114,0,111,0,115,0,111,0,102,0,116,0,46,0,87,0,105,0,110,0,100,0,111,0,119,0,115,0,46,0,83,0,121,0,115,0,116,0,101,0,109,0,67,0,111,0,109,0,112,0,97,0,116,0,105,0,98,0,108,0,101,0,44,0,112,0,114,0,111,0,99,0,101,0,115,0,115,0,111,0,114,0,65,0,114,0,99,0,104,0,105,0,116,0,101,0,99,0,116,0,117,0,114,0,101,0,61,0,34,0,120,0,56,0,54,0,34,0,44,0,112,0,117,0,98,0,108,0,105,0,99,0,75,0,101,0,121,0,84,0,111,0,107,0,101,0,110,0,61,0,34,0,54,0,53,0,57,0,53,0,98,0,54,0,52,0,49,0,52,0,52,0,99,0,99,0,102,0,49,0,100,0,102,0,34,0,44,0,116,0,121,0,112,0,101,0,61,0,34,0,119,0,105,0,110,0,51,0,50,0,34,0,44,0,118,0,101,0,114,0,115,0,105,0,111,0,110,0,61,0,34,0,54,0,46,0,48,0};

int main (void)
{
    //if(argc < 2) return(1);
	char * filename = "mustard2.png";
    /* create a reader */
    scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    /* obtain image data */
    int width = 0, height = 0;
    void *raw = NULL;

    /* declare LodePNG related vars */
    unsigned error;
    unsigned char* rawimage;

    unsigned char* png = 0;
    size_t pngsize;
    LodePNGState state;

    /* customize the state to give us 8bit Greyscale or Y800 expected by ZBAR*/
    lodepng_state_init(&state);
    state.info_raw.bitdepth = 8;
    state.info_raw.colortype = LCT_GREY;

    /* load the png file and get the rawimage data*/
    error = lodepng_load_file(&png, &pngsize, filename);
    if(!error) error = lodepng_decode(&rawimage, &width, &height, &state, png, pngsize);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
    free(png);

    lodepng_state_cleanup(&state);
    //free(image);

    unsigned char* rawimagebegin = (unsigned char*)rawimage;

    /* encode it back into a file to see if it's grayscale - uses only R values according
     * to lodepng documentation */
    //unsigned error2 = lodepng_encode(&png, &pngsize, rawimage, width, height, &state);
    //if(!error2) lodepng_save_file(png, pngsize, "encode.png");

    /* print raw data for debug */
    //rawimage[width*height] = '0';
    int i=0;
    printf("width = %d\n", width);
    printf("height = %d\n", height);
    printf("image size = %d", width*height);
    printf("\nraw string:\n");
    for (i=0; i<1140; i++){
    	printf("%#02x,", *rawimagebegin);
    	//printf("%d,", *rawimagebegin);
    	rawimagebegin++;
    }
    printf("\ndone\r\n");

    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, zbar_fourcc('G','R','E','Y'));
    zbar_image_set_size(image, width, height);
    //zbar_image_set_data(image, teststring, width * height, zbar_image_free_data);
    zbar_image_set_data(image, rawimage, width * height, zbar_image_free_data);

    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);

    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* print the results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",
               zbar_get_symbol_name(typ), data);
    }

    /* clean up */
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);
    //free(rawimage);
    return(0);
}
