#ifndef FLACPROCESS_H
#define FLACPROCESS_H

#include <FLAC/stream_decoder.h>

#include "list.h"

#include "common.h"

typedef struct {
    u8* data;
    u32 dataSize;
    u32 decoderReadPosition;
    ListData decodedSamples[1];
    u32 sampleRate;
    u16 channels;
} __I_DecodeUserData;

static FLAC__StreamDecoderReadStatus __I_FLAC_Callback_Read(
    const FLAC__StreamDecoder* decoder, 
    u8* buffer, 
    u64* bytes, 
    void* _userData
) {
    (void)decoder;
    if (_userData == NULL)
        panic("__I_FLAC_Callback_Read: _userData is NULL");

    __I_DecodeUserData* userData = (__I_DecodeUserData*)_userData;

    // No more data left
    if (userData->decoderReadPosition >= userData->dataSize) {
        *bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }

    u64 remaining = userData->dataSize - userData->decoderReadPosition;
    u64 toCopy = (*bytes > remaining) ? remaining : *bytes;

    memcpy(buffer, userData->data + userData->decoderReadPosition, toCopy);

    userData->decoderReadPosition += toCopy;
    *bytes = toCopy;

    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderWriteStatus __I_FLAC_Callback_Write(
    const FLAC__StreamDecoder* decoder, 
    const FLAC__Frame* frame, 
    const FLAC__int32* const buffer[], 
    void* _userData
) {
    (void)decoder;
    if (_userData == NULL)
        panic("__I_FLAC_Callback_Write: _userData is NULL");

    ListData* list = ((__I_DecodeUserData*)_userData)->decodedSamples;
    u32 channels = frame->header.channels;
    u32 blocksize = frame->header.blocksize;

    s16* interleaved = (s16*)malloc(blocksize * channels * sizeof(s16));
    if (interleaved == NULL)
        panic("__I_FLAC_Callback_Write: malloc failed");

    for (unsigned i = 0; i < blocksize; ++i)
        for (unsigned c = 0; c < channels; ++c)
            interleaved[i * channels + c] = buffer[c][i];

    ListAddRange(list, (void*)interleaved, blocksize * channels);

    free(interleaved);
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void __I_FLAC_Callback_Metadata(
    const FLAC__StreamDecoder* decoder,
    const FLAC__StreamMetadata* metadata,
    void* _userData
) {
    (void)decoder;
    if (_userData == NULL)
        panic("__I_FLAC_Callback_Metadata: _userData is NULL");

    __I_DecodeUserData* userData = (__I_DecodeUserData*)_userData;

    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        const FLAC__StreamMetadata_StreamInfo* streamInfo = &metadata->data.stream_info;

        userData->sampleRate = streamInfo->sample_rate;
        userData->channels = streamInfo->channels;

        ListReserve(userData->decodedSamples, streamInfo->total_samples);
    }
}


static void __I_FLAC_Callback_Error(
    const FLAC__StreamDecoder* decoder, 
    FLAC__StreamDecoderErrorStatus status, 
    void* _userData
) {
    (void)decoder;

    panic("__I_FLAC_Callback_Error: decoder err: %s", FLAC__StreamDecoderErrorStatusString[status]);
}

// Returns PCM16 samples
ListData FlacDecode(u8* flacData, u32 dataSize, u32* sampleRateOut, u16* channelsOut) {
    if (*(u32*)flacData != IDENTIFIER_TO_U32('f', 'L', 'a', 'C'))
        panic("FlacDecode: fLaC magic is nonmatching");

    FLAC__StreamDecoder* decoder = FLAC__stream_decoder_new();
    if (!decoder)
        panic("FlacDecode: FLAC__stream_decoder_new failed");
    
    __I_DecodeUserData userData;
    userData.data = flacData;
    userData.dataSize = dataSize;
    userData.decoderReadPosition = 0;
    userData.sampleRate = 0;
    userData.channels = 0;
    ListInit(userData.decodedSamples, sizeof(s16), 0);

    FLAC__stream_decoder_init_stream(
        decoder,

        __I_FLAC_Callback_Read,
        NULL, NULL, NULL, NULL,
        __I_FLAC_Callback_Write,
        __I_FLAC_Callback_Metadata,
        __I_FLAC_Callback_Error,

        (void*)&userData
    );

    if (!FLAC__stream_decoder_process_until_end_of_stream(decoder)) {
        FLAC__stream_decoder_delete(decoder);
        panic("FlacDecode: Decoding failed ( FLAC__stream_decoder_process_until_end_of_stream(decoder) == 0 )");
    }

    FLAC__stream_decoder_delete(decoder);

    if (sampleRateOut != NULL)
        *sampleRateOut = userData.sampleRate;
    if (channelsOut != NULL)
        *channelsOut = userData.channels;

    return *userData.decodedSamples;
}

#endif // FLACPROCESS_H
