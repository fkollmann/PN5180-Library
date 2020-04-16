#include <Arduino.h>
#include "NDEFReader.h"
#include "PN5180ISO15693.h"

ISO15693ErrorCode NDEFReader::readNextByte(uint8_t& data) {
    // read metadata
    ISO15693ErrorCode rc;

    if (_blockSize <= 0) {
        rc = _nfc->getSystemInfo(_uid, &_blockSize, &_blockCount);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
    }

    // read next block?
    if (_blockDataPos >= _blockSize) {
        _blockIndex++;

        if (_blockIndex >= _blockCount) {
            // done with final block
            return EC_NO_BLOCK_LEFT;
        }

        rc = _nfc->readSingleBlock(_uid, _blockIndex, _blockData, _blockSize);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        _blockDataPos = 0;
    }

    // return next byte
    data = _blockData[_blockDataPos];

    _blockDataPos++;

    return ISO15693_EC_OK;
}

ISO15693ErrorCode NDEFReader::readMessageAsText(String& text) {
    // read first byte
    byte firstByte;
    ISO15693ErrorCode rc;

    rc = readNextByte(firstByte);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }

    // check if TLV container
    // (this check is sufficient as 0xe1 is an invalid NDEF header)
    if (firstByte == 0xe1) {
        uint8_t version;
        rc = readNextByte(version);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
        if ((version & 0xf0) != 0x40) {
            return EC_TLV_FORMAT_ERROR;
        }

        // skip page count
        // skip access level
        skipNextByte(2);
        
        uint8_t contentType;
        rc = readNextByte(contentType);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
        if (contentType != 0x03 /* NDEF */) {
            // not a NDEF message
            return EC_NO_BLOCK_LEFT;
        }

        // skip content length
        // (will be taken from NDEF not the TLV)
        skipNextByte();

        // read first byte of content/NDEF message
        rc = readNextByte(firstByte);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
    }

    // decode tnf - first byte is tnf with bit flags
    //bool mb = (tnf_byte & 0x80) == 0x80;
    //bool me = (firstByte & 0x40) == 0x40;
    //bool cf = (tnf_byte & 0x20) == 0x20;
    bool sr = (firstByte & 0x10) == 0x10;
    bool il = (firstByte & 0x08) == 0x08;
    uint8_t tnf = (firstByte & 0x07);

    // currently, only well known
    // types are supported
    if (tnf != 0x01) {
        return EC_NDEF_FORMAT_ERROR;
    }

    // get type length
    uint8_t typeLength;

    rc = readNextByte(typeLength);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }

    // get payload length
    uint32_t payloadLength;

    if (sr) {
        uint8_t pl1;

        rc = readNextByte(pl1);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        payloadLength = pl1;
    } else {
        uint8_t pl1, pl2, pl3, pl4;

        rc = readNextByte(pl1);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        rc = readNextByte(pl2);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        rc = readNextByte(pl3);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        rc = readNextByte(pl4);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }

        payloadLength =
                (static_cast<uint32_t>(pl1)   << 24)
            | (static_cast<uint32_t>(pl2) << 16)
            | (static_cast<uint32_t>(pl3) << 8)
            |  static_cast<uint32_t>(pl4);
    }

    // skip the id length
    uint8_t idLength = 0;

    if (il) {
        rc = readNextByte(idLength);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
    }

    // read type information
    // currently, only text is supported
    if (typeLength != 1) {
        return EC_NDEF_FORMAT_ERROR;
    }

    uint8_t type;

    rc = readNextByte(type);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }
    if (type != 'T') {
        return EC_NDEF_FORMAT_ERROR;
    }

    // skip the id
    skipNextByte(idLength);

    // skip text language code
    uint8_t langCodeLength;
    rc = readNextByte(langCodeLength);
    if (ISO15693_EC_OK != rc) {
        return rc;
    }

    skipNextByte(langCodeLength);

    payloadLength--;
    payloadLength -= langCodeLength;

    // read payload
    uint8_t data[payloadLength + 1];

    for (uint32_t i = 0; i < payloadLength; i++) {
        rc = readNextByte(data[i]);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
    }

    // done
    data[payloadLength] = 0;

    text = std::move(String((const char*)data));

    return ISO15693_EC_OK;
}

ISO15693ErrorCode NDEFReader::skipNextByte(uint8_t count) {
    ISO15693ErrorCode rc;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t t;

        rc = readNextByte(t);
        if (ISO15693_EC_OK != rc) {
            return rc;
        }
    }

    return ISO15693_EC_OK;
}

NDEFReader::NDEFReader(PN5180ISO15693& nfc, uint8_t *uid) {
    assert(uid != nullptr);

    _nfc = &nfc;
    _uid = uid;
}

NDEFReader::~NDEFReader() {

}
