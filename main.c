#include <stdio.h>
#include <stdlib.h>
#include "opus.h"
#include <sys/time.h>
#include <unistd.h>

#define DEFAULT_SAMPLERATEINHz 16000
#define DEFAULT_CHANNELCONFIG 1

#define MAX_PACKET 1500

#include "lame.h"
int main(int argc, char **argv)
{
    /*int a[10] = {1,2,3,4,5};
    int *ptr = (int*)(&a+1);
    printf("%d,%d\n",*(a+1),*(ptr-1));
    printf("sizeof(a) = %d,sizeof(&a)= %d,sizeof(*a)=%d,sizeof(a[0]) = %d,sizeof(ptr)=%d\n",
    sizeof(a), sizeof(&a),sizeof(*a),sizeof(a[0]),sizeof(ptr));*/
    struct timeval tv;
    gettimeofday(&tv,NULL);
    long start = tv.tv_sec*1000000 + tv.tv_usec;
    FILE *fin;
    FILE *fout;
    short *in = NULL;
    short *out = NULL;

    unsigned char *fbytes = NULL;
    int max_frame_size = DEFAULT_SAMPLERATEINHz*2;

    int error;
    size_t num_read;
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <raw opus input> <mp3 output>\n", argv[0]);
        return 1;
    }
    fin = fopen(argv[1], "rb");
    if (!fin)
    {
        fprintf(stderr, "cannot open input file: %s\n", argv[1]);
        return 1;
    }
    fout = fopen(argv[2], "wb");
    if (!fout)
    {
        fprintf(stderr, "cannot open output file: %s\n", argv[2]);
        return 1;
    }
    int sampleRateInHz = DEFAULT_SAMPLERATEINHz;
    int channelConfig = DEFAULT_CHANNELCONFIG;

    
    
    /* We need to allocate for 16-bit PCM data, but we store it as unsigned char. */


    OpusDecoder *pOpusDec = opus_decoder_create(sampleRateInHz, channelConfig, &error);
    if (error != OPUS_OK)
    {
        fprintf(stderr, "Cannot create decoder: %s\n", opus_strerror(error));
        goto failure;
    }
    #define WRITE_BUFFER_SIZE 520
    #define READ_BUFFER_SIZE 40
    #define DECODE_OUTPUT_SIZE 320
    fbytes = (unsigned char*)malloc(WRITE_BUFFER_SIZE);
    out = (short *)malloc(640 * sizeof(short));
    lame_global_flags *lame = NULL;
    lame = lame_init();
    lame_set_in_samplerate(lame, sampleRateInHz);
    lame_set_num_channels(lame, channelConfig);//输入流的声道
    lame_set_out_samplerate(lame, sampleRateInHz);
    lame_set_brate(lame, 16);
    lame_set_quality(lame, 7);
    lame_init_params(lame);
    int i = 0;
    while (1)
    {
    gettimeofday(&tv,NULL);
    long startDec = tv.tv_sec*1000000 + tv.tv_usec;
        unsigned char data[READ_BUFFER_SIZE];
        num_read = fread(data, 1, READ_BUFFER_SIZE, fin);
        
        if (num_read > 0)
        {
            //printf("count = %d\n", i++);
            int output_samples = opus_decode(pOpusDec, data, num_read, out, DECODE_OUTPUT_SIZE, 0);
            int result = lame_encode_buffer(lame, out, out,output_samples, fbytes, WRITE_BUFFER_SIZE);
            if (fwrite(fbytes, 1, result, fout) != (unsigned)(result)){
                printf("write error,output_samples = %d, result = %d\n",output_samples,result);
                fprintf(stderr, "111 Error writing.\n");
                goto failure;
            }
        }
        else{
            int result = lame_encode_flush(lame, fbytes, WRITE_BUFFER_SIZE);
            printf("result:%d\n",result);
            if(result >= 0){
                if (fwrite(fbytes, 1, result, fout) != (unsigned)(result)){
                    fprintf(stderr, "222 Error writing.\n");
                    goto failure;
                }
            }
            
            lame_close(lame);
            break;
        }
        gettimeofday(&tv,NULL);
        long endDec = tv.tv_sec*1000000 + tv.tv_usec;   
        printf("decode %d speend:%lld\n", i++, endDec - startDec);    
    }
    fclose(fout);
failure:
    /*opus_encoder_destroy(enc);
    opus_decoder_destroy(dec);
    free(data[0]);
    free(data[1]);
    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);
    free(in);
    free(out);
    free(fbytes);*/
    gettimeofday(&tv,NULL);
    long end = tv.tv_sec*1000000 + tv.tv_usec;
    printf("spend  = %lld\n", end - start);
return 0;
}