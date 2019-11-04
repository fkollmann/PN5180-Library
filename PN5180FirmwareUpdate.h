#ifndef PN5180FIRMWAREUPDATE_H
#define PN5180FIRMWAREUPDATE_H

#include "PN5180.h"

enum PN5180FWState {
  FWERR_OK                      = 0x00, /**< Success, no error. */
  FWERR_INVALID_ADDR            = 0x01,	/**< Trying to read or write an invalid address. */
  FWERR_GENERIC_ERROR           = 0x02,	/**< Generic error occurred. */
  FWERR_UNKNOWN_CMD             = 0x0B,	/**< Unknown Command. */
  FWERR_ABORTED_CMD             = 0x0C,	/**< Chunk sequence is large . */
  FWERR_PLL_ERROR               = 0x0D,	/**< PLL configuration is wrong, flash not activated. */
  FWERR_ADDRESS_RANGE_OVERFLOW  = 0x1E,	/**< Write/read performed outside of address range. */
  FWERR_BUFFER_OVERFLOW         = 0x1F,	/**< Buffer overflow error. */
  FWERR_MEMORY_BUSY             = 0x20,	/**< EEPROM Memory operation in progress. */
  FWERR_SIGNATURE_ERROR         = 0x21,	/**< Signature mismatch. */
  FWERR_FIRMWARE_VERSION_ERROR  = 0x24,	/**< trying to write same firmware version. */
  FWERR_PROTOCOL_ERROR          = 0x28,	/**< Download protocol error. */
  FWERR_DEGRADED                = 0x2A,	/**< EEPROM corruption. */

  FWERR_MAGICNO                 = 0xF0  /**< Firmware image wrong magic number. */
};

class PN5180FirmwareUpdate : public PN5180 {
protected:
  const uint8_t PN5180_REQ;

public:

    PN5180FirmwareUpdate(uint8_t SSpin, uint8_t BUSYpin, uint8_t RSTpin, uint8_t REQpin);

    void begin();
    void doUpdate();
    void end();

    void getFirmwareVersion(uint8_t &major, uint8_t &minor);

    bool checkImage(const uint8_t *img, size_t imgLen, uint8_t &major, uint8_t &minor);
    PN5180FWState uploadImage(const uint8_t *img, size_t imgLen);

private:

  bool transceiveCommandDownloadMode(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer = nullptr, size_t recvBufferLen = 0);

};

#endif
