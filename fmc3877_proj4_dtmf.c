/******************************************************************************
* File Name: fmc3877_proj4_dtmf.c
*
* Author: Farhan Chowdhury
*
* Created for RIT CMPR271 on: 11/16/2025
*
* Purpose: Takes arguments from the command line and generates a Windows
*          wave format file composed of DTMF tones.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cabeee_wavefile.h"    // specific for dealing with .wav files
#include "fmc3877_proj4_dtmf.h" // prototypes for functions written for this project

/******************************************************************************
* Function Name: main()
*
* Purpose: Validates the input, then builds WAVE header, generates 
*          DTMF samples, and finally writes the .wav file.
*
******************************************************************************/

int main(int argc, const char *argv[])
{
    // 6. Validates the number of arguments
    if (argc != 4)
    {
        show_usage();
        return -1;
    }

    // 8. Validates the tone length
    double toneLength = atof(argv[2]);
    if (toneLength < 0.1 || toneLength > 1.0)
    {
        show_usage();
        return -1;
    }

    // 7. Validates the  digit string
    const char *phoneString = argv[3];
    int i = 0;
    while (phoneString[i] != '\0')
    {
        char c = phoneString[i];
        if (!((c >= '0' && c <= '9') || c == '-' || c == '*' || c == '#'))
        {
            show_usage();
            return -1;
        }
        i++;
    }
    int numDigits = i;

    // 10. Allocates and fills the WAVEFILE header
    WAVEFILE *headerPtr = malloc(sizeof(WAVEFILE));
    if (headerPtr == NULL)
    {
        return -1;
    }

    headerPtr->ChunkID[0] = 'R';
    headerPtr->ChunkID[1] = 'I';
    headerPtr->ChunkID[2] = 'F';
    headerPtr->ChunkID[3] = 'F';

    headerPtr->Format[0] = 'W';
    headerPtr->Format[1] = 'A';
    headerPtr->Format[2] = 'V';
    headerPtr->Format[3] = 'E';

    headerPtr->SubChunk1ID[0] = 'f';
    headerPtr->SubChunk1ID[1] = 'm';
    headerPtr->SubChunk1ID[2] = 't';
    headerPtr->SubChunk1ID[3] = ' ';

    headerPtr->SubChunk1Size = 16;
    headerPtr->AudioFormat   = PCM_FORMAT;
    headerPtr->NumChannels   = NUM_CHANNELS;
    headerPtr->SampleRate    = SAMPLE_RATE;
    headerPtr->BitsPerSample = BYTES_PER_SAMPLE * 8;
    headerPtr->BlockAlign    = NUM_CHANNELS * BYTES_PER_SAMPLE;
    headerPtr->ByteRate      = SAMPLE_RATE * headerPtr->BlockAlign;

    headerPtr->SubChunk2ID[0] = 'd';
    headerPtr->SubChunk2ID[1] = 'a';
    headerPtr->SubChunk2ID[2] = 't';
    headerPtr->SubChunk2ID[3] = 'a';

    // 11. Compute sizes and allocate sample buffer 
    int samplesPerDigit = (int)(SAMPLE_RATE * toneLength);
    int totalSamples    = samplesPerDigit * numDigits;
    int totalBytes      = totalSamples * BYTES_PER_SAMPLE;

    headerPtr->SubChunk2Size = (uint32_t) totalBytes;
    headerPtr->ChunkSize =
        4 + (8 + headerPtr->SubChunk1Size) +
            (8 + headerPtr->SubChunk2Size);

    int16_t *dataSamplesPtr = malloc((size_t) totalBytes);
    if (dataSamplesPtr == NULL)
    {
        free(headerPtr);
        return -1;
    }

    // 12. Generates the samples 
    int sampleIndex = 0;

    for (i = 0; i < numDigits; i++)
    {
        char c = phoneString[i];
        int f1, f2;
        get_freqs(c, &f1, &f2);

        for (int n = 0; n < samplesPerDigit; n++)
        {
            double sampleValue;

            if (f1 == 0 && f2 == 0)
            {
                sampleValue = 0.0;
            }
            else
            {
                const double PI = 3.141592653589793;
                sampleValue =
                    AMPLITUDE *
                    (sin(2.0 * PI * f1 * n / SAMPLE_RATE) +
                     sin(2.0 * PI * f2 * n / SAMPLE_RATE));
            }

            dataSamplesPtr[sampleIndex] = (int16_t) sampleValue;
            sampleIndex++;
        }
    }

    // 13. Writes the header and data
    FILE *fp = fopen(argv[1], "wb");
    if (fp == NULL)
    {
        free(headerPtr);
        free(dataSamplesPtr);
        return -1;
    }

    fwrite(headerPtr, sizeof(WAVEFILE), 1, fp);         // First fwrite 
    fwrite(dataSamplesPtr, (size_t) totalBytes, 1, fp); // Second fwrite 

    fclose(fp);

    // 15. Frees memory
    free(headerPtr);
    free(dataSamplesPtr);

    return 0;
}

/******************************************************************************
* Function Name: get_freqs()
*
* Purpose: For a given DTMF character, returns its two frequencies.
*
* Arguments: None
*
* Returns: Nothing
*
******************************************************************************/

void get_freqs(char c, int *f1, int *f2)
{
    switch (c)
    {
        case '1': *f1 = 697;  *f2 = 1209; break;
        case '2': *f1 = 697;  *f2 = 1336; break;
        case '3': *f1 = 697;  *f2 = 1477; break;

        case '4': *f1 = 770;  *f2 = 1209; break;
        case '5': *f1 = 770;  *f2 = 1336; break;
        case '6': *f1 = 770;  *f2 = 1477; break;

        case '7': *f1 = 852;  *f2 = 1209; break;
        case '8': *f1 = 852;  *f2 = 1336; break;
        case '9': *f1 = 852;  *f2 = 1477; break;

        case '*': *f1 = 941;  *f2 = 1209; break;
        case '0': *f1 = 941;  *f2 = 1336; break;
        case '#': *f1 = 941;  *f2 = 1477; break;

        case '-': *f1 = 0;    *f2 = 0;    break; 

        default:  *f1 = 0;    *f2 = 0;    break;
    }
}

/******************************************************************************
* Function Name: show_usage()
*
* Purpose: Shows the correct program usage format.
*
* Arguments: None
*
* Returns: Nothing
*
******************************************************************************/

void show_usage(void)
{
    printf("\nUsage: ./fmc3877_proj4_dtmf.exe X Y Z\n\n");
    printf(" Where X is the name of the Wave file to create\n");
    printf(" Where Y is the duration (in seconds) of each dtmf tone,\n");
    printf("         valid range is 0.1 - 1.0\n");
    printf(" Where Z is the string of digits to create tones for, valid\n");
    printf("         digits are 0-9, #, * and - for a duration of silence\n");
    printf("\nExample: ./fmc3877_proj4_dtmf.exe wavy.wav 0.3 123-3831\n\n");
}