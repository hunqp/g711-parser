#include <bits/stdc++.h>
#include <sys/stat.h>
#include "g711_parser.h"

static const bool AMPLIFY_PCM = true;

static void aLawDecodeTest(std::string aLawURL, std::string pcmURL);
static void pcmEncodeTest(std::string pcmURL, std::string aLawURL);

int main() {
    const std::string ALAW_ROOT_URL = "audio_voices.aLaw";
    std::string AUDIO_DECODE_URL    = "audio_voices.pcm";
    std::string AUDIO_ENCODE_URL    = "audio_voices.g711a";

    aLawDecodeTest(ALAW_ROOT_URL, AUDIO_DECODE_URL);
    pcmEncodeTest(AUDIO_DECODE_URL, AUDIO_ENCODE_URL);

    return 0;
}

void aLawDecodeTest(std::string aLawURL, std::string pcmURL) {
    FILE *FILE_SRC = fopen(aLawURL.c_str(), "rb");
	FILE *FILE_DES = fopen(pcmURL.c_str(), "wb");

    if (FILE_SRC == NULL || FILE_DES == NULL) {
        std::cout << "Can't open " << aLawURL << std::endl;
        std::cout << "Can't open " << pcmURL << std::endl;
        return;
    }

    struct stat STAT;
    stat(aLawURL.c_str(), &STAT);
	
	int read = 0;
    int dataLen = STAT.st_size;

	uint8_t aLawBuffer[dataLen + 1];
	uint8_t pcmBuffer[(dataLen * 2) + 1];

	memset(aLawBuffer, 0, sizeof(aLawBuffer));
    memset(pcmBuffer,  0, sizeof(pcmBuffer));

	read = fread(aLawBuffer, 1, dataLen, FILE_SRC);
	if (read) {
		int ret = g711::aLaw2PCM(aLawBuffer, pcmBuffer, read);
        std::vector<uint8_t> amplifiedPCMBuffer;
        if (AMPLIFY_PCM) {
            auto amplifiedPCMBuffer = g711::makePCMSoundLouderV1(pcmBuffer, sizeof(pcmBuffer));
            std::cout << "PCM Amplified Buffer size " << amplifiedPCMBuffer.size() << std::endl;
            memcpy(pcmBuffer, amplifiedPCMBuffer.data(), sizeof(pcmBuffer));
        }

        if (ret != G711_RC_FAILURE) {
            fwrite(pcmBuffer, 1, ret, FILE_DES);
        }
        else {
            std::cout << "Write " << pcmURL << " failed" << std::endl; 
        }
	}
    else {
        std::cout << "Can't read " << aLawURL << std::endl;
    }
    
	fclose(FILE_SRC);
	fclose(FILE_DES);

    stat(pcmURL.c_str(), &STAT);
    std::cout << "Create PCM\t" << pcmURL << "\tsize " << STAT.st_size << std::endl;
}

void pcmEncodeTest(std::string pcmURL, std::string aLawURL) {
    FILE *FILE_SRC = fopen(pcmURL.c_str(), "rb");
    FILE *FILE_DES = fopen(aLawURL.c_str(), "wb");

    if (FILE_SRC == NULL || FILE_DES == NULL) {
        std::cout << "Can't open " << pcmURL << std::endl;
        std::cout << "Can't open " << aLawURL << std::endl;
        return;
    }

    struct stat STAT;
    stat(pcmURL.c_str(), &STAT);

    int read = 0;
    int dataLen = STAT.st_size;

    uint8_t aLawBuffer[dataLen +1];
	uint8_t pcmBuffer[dataLen + 1];
    memset(aLawBuffer, 0, sizeof(aLawBuffer));
    memset(pcmBuffer,  0, sizeof(pcmBuffer));
	
	read = fread(aLawBuffer, 1, dataLen, FILE_SRC);

	if (read) {
		int ret = g711::PCM2ALaw(aLawBuffer, pcmBuffer, read);
        if (ret != G711_RC_FAILURE) {
            fwrite(pcmBuffer, 1, ret, FILE_DES);
        }	
	}

    fclose(FILE_SRC);
	fclose(FILE_DES);

    stat(aLawURL.c_str(), &STAT);
    std::cout << "Create ALAW\t" << aLawURL << "\tsize " << STAT.st_size << std::endl;
}