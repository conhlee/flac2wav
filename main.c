#include <stdlib.h>

#include "files.h"

#include "list.h"

#include "flacProcess.h"
#include "wavProcess.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: flac2wav <input flac> [output wav]\n");
        return 1;
    }

    int wavOutputSpecified = argc >= 3;

    char* flacPath = argv[1];
    char* wavPath = wavOutputSpecified ? argv[2] : strcat(strdup(flacPath), ".wav");
    if (wavOutputSpecified)
        wavPath = argv[2];
    else {
        wavPath = (char*)malloc(strlen(flacPath) + STR_LIT_LEN(".wav") + 1);
        sprintf(wavPath, "%s.wav", flacPath);
    }

    FileHandle fileHndl = FileCreateHandle(flacPath);

    u32 sampleRate;
    u16 channelCount;

    ListData samples = FlacDecode(fileHndl.data_u8, fileHndl.size, &sampleRate, &channelCount);

    FileDestroyHandle(fileHndl);

    printf("sampleCount=%lu sampleRate=%u, channelCount=%u\n", samples.elementCount, sampleRate, (unsigned)channelCount);

    fileHndl = WavBuild(samples.data, samples.elementCount, sampleRate, channelCount);

    FileWriteHandle(fileHndl, wavPath);

    LOG_OK;

    if (!wavOutputSpecified)
        free(wavPath);

    FileDestroyHandle(fileHndl);
}