#include <stdlib.h>

#include "files.h"

#include "list.h"

#include "flacProcess.h"
#include "wavProcess.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Input file arg missing\n");
        return 1;
    }

    char* flacPath = argv[1];

    FileHandle fileHndl = FileCreateHandle(flacPath);

    u32 sampleRate;
    u16 channelCount;

    ListData samples = FlacDecode(fileHndl.data_u8, fileHndl.size, &sampleRate, &channelCount);

    FileDestroyHandle(fileHndl);

    printf("sampleCount=%lu sampleRate=%u, channelCount=%u\n", samples.elementCount, sampleRate, (unsigned)channelCount);

    fileHndl = WavBuild(samples.data, samples.elementCount, sampleRate, channelCount);
    FileWriteHandle(fileHndl, strcat(flacPath, ".wav"));

    FileDestroyHandle(fileHndl);
}