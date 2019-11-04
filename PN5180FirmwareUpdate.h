#ifndef PN5180FIRMWAREUPDATE_H
#define PN5180FIRMWAREUPDATE_H

#include "PN5180.h"

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
    bool uploadImage(const uint8_t *img, size_t imgLen);

private:

  bool transceiveCommandDownloadMode(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer = nullptr, size_t recvBufferLen = 0);

};

#endif
