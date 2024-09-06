#include "g711_parser.h"

namespace g711 {

static int16_t segmentEnd[8] = { 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };

static uint16_t search(uint16_t val, int16_t *table, uint16_t size);
static int16_t aLawToLinear(uint8_t aByte);
static int16_t uLawToLinear(uint8_t uByte);
static uint8_t linearToULaw(int16_t pcm);
static uint8_t linearToALaw(int16_t pcm);

uint16_t PCM2ALaw(uint8_t *pcmBuffer, uint8_t *aLawBuffer, uint16_t dataLen) {
	if (pcmBuffer == NULL && aLawBuffer == NULL && dataLen == 0) {
		return G711_RC_FAILURE;
	}

	return aLawEncode((uint8_t*)aLawBuffer, (int16_t*)pcmBuffer, dataLen / 2);
}

uint16_t PCM2ULaw(uint8_t *pcmBuffer, uint8_t *uLawBuffer, uint16_t dataLen) {
	if (pcmBuffer == NULL && uLawBuffer == NULL && dataLen == 0) {
		return G711_RC_FAILURE;
	}

	return uLawEncode((uint8_t*)uLawBuffer, (int16_t*)pcmBuffer, dataLen / 2);; 
}

uint16_t aLaw2PCM(uint8_t *aLawBuffer, uint8_t *pcmBuffer, uint16_t dataLen) {
	if (pcmBuffer == NULL && aLawBuffer == NULL && dataLen == 0) {
		return G711_RC_FAILURE;
	}

	return aLawDecode((int16_t*)pcmBuffer, (uint8_t*)aLawBuffer, dataLen);
}

uint16_t uLaw2PCM(uint8_t *uLawBuffer, uint8_t *pcmBuffer, uint16_t dataLen) {
	if (pcmBuffer == NULL && uLawBuffer == NULL && dataLen == 0) {
		return G711_RC_FAILURE;
	}

	return uLawDecode((int16_t*)pcmBuffer, (uint8_t*)uLawBuffer, dataLen);
}

uint8_t linearToALaw(int16_t pcm) {
	int16_t mask, seg;
	uint8_t	aLaw;

	if (pcm >= 0) {
		mask = 0xD5;
	}
	else {
		mask = 0x55;
		pcm = -pcm - 8;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm, segmentEnd, 8);

	/* Combine the sign, segment, and quantization bits. */
	if (seg >= 8) {
		return (0x7F ^ mask);
	}
	
	aLaw = seg << SEG_SHIFT;
	if (seg < 2) {
		aLaw |= (pcm >> 4) & QUANT_MASK;
	}
	else {
		aLaw |= (pcm >> (seg + 3)) & QUANT_MASK;
	}

	return (aLaw ^ mask);
}

uint8_t linearToULaw(int16_t pcm) {
	int16_t mask, seg;
	uint8_t	uLaw;

	if (pcm < 0) {
		pcm = BIAS - pcm;
		mask = 0x7F;
	}
	else {
		pcm += BIAS;
		mask = 0xFF;
	}

	/* Convert the scaled magnitude to segment number. */
	seg = search(pcm, segmentEnd, 8);

	/* Combine the sign, segment, quantization bits and complement the code word. */
	if (seg >= 8) {
		return (0x7F ^ mask);
	}
	
	uLaw = (seg << 4) | ((pcm >> (seg + 3)) & 0xF);

	return (uLaw ^ mask);
}

uint16_t aLawDecode(int16_t pcmBuffer[], const uint8_t aLawBuffer[], uint16_t aLawBufferSize) {
	uint16_t i;
	uint16_t samples;
	uint8_t code;
	uint16_t sl;

	for (samples = i = 0; ; ) {
		if (i >= aLawBufferSize) {
			break;
		}
		code = aLawBuffer[i++];

		sl = aLawToLinear(code);

		pcmBuffer[samples++] = (int16_t) sl;
	}
	return (samples * 2);
}

uint16_t uLawDecode(int16_t pcmBuffer[], const uint8_t uLawBuffer[], uint16_t uLawBufferSize) {
	uint16_t i;
	uint16_t samples;
	uint8_t code;
	uint16_t sl;

	for (samples = i = 0;;) {
		if (i >= uLawBufferSize) {
			break;
		}
		code = uLawBuffer[i++];

		sl = uLawToLinear(code);

		pcmBuffer[samples++] = (int16_t) sl;
	}
	return samples*2;
}

uint16_t aLawEncode(uint8_t aLawBuffer[], const int16_t pcmBuffer[], uint16_t pcmBufferSize) {
    for (uint16_t id = 0;  id < pcmBufferSize; ++id) {
        aLawBuffer[id] = linearToALaw(pcmBuffer[id]);
    }

    return pcmBufferSize;
}

uint16_t uLawEncode(uint8_t uLawBuffer[], const int16_t pcmBuffer[], uint16_t pcmBufferSize) {
    for (uint16_t id = 0;  id < pcmBufferSize; ++id) {
        uLawBuffer[id] = linearToULaw(pcmBuffer[id]);
    }

    return pcmBufferSize;
}

uint16_t search(uint16_t val, int16_t *table, uint16_t size) {
	uint16_t id;

	for (id = 0; id < size; id++) {
		if (val <= *table++) {
            return (id);
        }
	}

	return (size);
}

int16_t aLawToLinear(uint8_t aByte) {
	int	t;
	int	seg;

	aByte ^= 0x55;

	t = (aByte & QUANT_MASK) << 4;
	seg = ( (unsigned)aByte & SEG_MASK ) >> SEG_SHIFT;
	
	switch (seg) {
		case 0: t += 8;
		break;

		case 1: t += 0x108;
		break;

		default: {
			t += 0x108;
			t <<= seg - 1;
		}
		break;
	}

	return ((aByte & SIGN_BIT) ? t : -t);
}

int16_t uLawToLinear(uint8_t uByte) {
	int	t;

	uByte = ~uByte;
	t = ((uByte & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)uByte & SEG_MASK) >> SEG_SHIFT;

	return ((uByte & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

std::vector<uint8_t> makePCMSoundLouderV1(uint8_t *pcmBuffer, uint32_t pcmBufferSize, float factor) {
    std::vector<uint8_t> amplifiedPCMBuffer(pcmBufferSize);

    for (uint32_t id = 0; id < pcmBufferSize; ++id) {
        int16_t pcmSample = static_cast<int16_t>((pcmBuffer[id + 1] << 8) | pcmBuffer[id]);
        int32_t amplifiedSample = static_cast<int32_t>(pcmSample) * factor;

        if (amplifiedSample > INT16_MAX) {
            amplifiedSample = INT16_MAX;
        }
        else if (amplifiedSample < INT16_MIN) {
            amplifiedSample = INT16_MIN;
        }

        amplifiedPCMBuffer[id] = static_cast<uint8_t>(amplifiedSample & 0xFF);
        amplifiedPCMBuffer[++id] = static_cast<uint8_t>((amplifiedSample >> 8) & 0xFF);
    }

    return amplifiedPCMBuffer;
}

std::vector<int16_t> makePCMSoundLouderV2(uint8_t *pcmBuffer, uint32_t pcmBufferSize, float factor) {
	std::vector<int16_t> amplifiedPCMBuffer(pcmBufferSize / 2);

	for (uint32_t id = 0; id < pcmBufferSize; id += 2) {
		int16_t pcmSample = static_cast<int16_t>((pcmBuffer[id + 1] << 8) | pcmBuffer[id]);
		int32_t amplifiedSample = static_cast<int32_t>(pcmSample) * factor;

		if (amplifiedSample > INT16_MAX) {
			amplifiedSample = INT16_MAX;
		}
		else if (amplifiedSample < INT16_MIN) {
			amplifiedSample = INT16_MIN;
		}

		amplifiedPCMBuffer[id / 2] = static_cast<int16_t>(amplifiedSample);
	}

	return amplifiedPCMBuffer;
}

}