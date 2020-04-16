#ifndef NDEFREADER_H_734657
#define NDEFREADER_H_734657

#include "PN5180ISO15693.h"

class NDEFReader {

private:

    PN5180ISO15693 *_nfc;
    uint8_t *_uid;
    uint8_t _blockCount = 0;
    uint8_t _blockSize = 0;
    int16_t _blockIndex = -1;
    uint16_t _blockDataPos = UINT16_MAX;
    uint8_t _blockData[256];

private:

    ISO15693ErrorCode readNextByte(uint8_t& data);
    ISO15693ErrorCode skipNextByte(uint8_t count = 1);

public:

    ISO15693ErrorCode readMessageAsText(String& text);

public:

    NDEFReader& operator = (const NDEFReader&) = delete;

    NDEFReader(PN5180ISO15693& nfc, uint8_t *uid);
    NDEFReader(const NDEFReader&) = delete;
    ~NDEFReader();

};

#endif
