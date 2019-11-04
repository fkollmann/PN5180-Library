#include <Arduino.h>
#include "PN5180FirmwareUpdate.h"
#include "Debug.h"
#include "PN5180Firmware_4_0.h"

// SOURCE: https://www.nxp.com/downloads/en/software-support/SW4055.zip
static uint16_t phHal_Host_CalcCrc16(uint8_t* p, uint32_t dwLength)
{
  uint32_t i;
  uint16_t crc_new;
  uint16_t crc = 0xffffU;

  for (i = 0; i < dwLength; i++)
  {
    crc_new = (uint8_t)(crc >> 8) | (crc << 8);
    crc_new ^= p[i];
    crc_new ^= (uint8_t)(crc_new & 0xff) >> 4;
    crc_new ^= crc_new << 12;
    crc_new ^= (crc_new & 0xff) << 5;
    crc = crc_new;
  }

  return crc;
}

#define DOWNLOADMODE_SEND ((uint8_t)0x7f)
#define DOWNLOADMODE_RECV ((uint8_t)0xff)
#define DOWNLOADCMD_GET_VERSION ((uint8_t)0xf1)

PN5180FirmwareUpdate::PN5180FirmwareUpdate(uint8_t SSpin, uint8_t BUSYpin, uint8_t RSTpin, uint8_t REQpin)
  : PN5180(SSpin, BUSYpin, RSTpin), PN5180_REQ(REQpin) {

}

void PN5180FirmwareUpdate::begin() {
  PN5180::begin();

  pinMode(PN5180_REQ, OUTPUT);

  digitalWrite(PN5180_REQ, LOW);
}

bool PN5180FirmwareUpdate::transceiveCommandDownloadMode(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer, size_t recvBufferLen) {
  // inject crc16
  uint8_t crc16[2];

  {
    auto r = phHal_Host_CalcCrc16(sendBuffer, sendBufferLen);

    crc16[0] = (r >> 8) & 0xff;
    crc16[1] = r  & 0xff;
  }

  // debug output
#ifdef DEBUG
  PN5180DEBUG(F("Sending SPI frame: '"));
  PN5180DEBUG(formatHex(DOWNLOADMODE_SEND));
  PN5180DEBUG(" ");
  for (size_t i=0; i<sendBufferLen; i++) {
    if (i>0) PN5180DEBUG(" ");
    PN5180DEBUG(formatHex(sendBuffer[i]));
  }
  PN5180DEBUG(" ");
  PN5180DEBUG(formatHex(crc16[0]));
  PN5180DEBUG(" ");
  PN5180DEBUG(formatHex(crc16[1]));
  PN5180DEBUG("'\n");
#endif

  // 0.
  while (LOW != digitalRead(PN5180_BUSY)); // wait until busy is low
  // 1.
  digitalWrite(PN5180_NSS, LOW); delay(2);
  // 2.
  SPI.transfer(DOWNLOADMODE_SEND);

  for (size_t i=0; i<sendBufferLen; i++) {
    SPI.transfer(sendBuffer[i]);
  }

  SPI.transfer(crc16[0]);
  SPI.transfer(crc16[1]);

  // 3.
  digitalWrite(PN5180_NSS, HIGH); delay(1);

  // check, if write-only
  if ((0 == recvBuffer) || (0 == recvBufferLen)) return true;
  PN5180DEBUG(F("Receiving SPI frame...\n"));

  // 1.
  while(HIGH != digitalRead(PN5180_BUSY));  // wait until BUSY is high

  // 2.
  digitalWrite(PN5180_NSS, LOW); delay(2);

  // 3.
  // check the transfer direction
  readBuffer[0] = SPI.transfer(0xff);
  if (readBuffer[0] != DOWNLOADMODE_RECV) {
      digitalWrite(PN5180_NSS, LOW);
      return false;
  }

  // read content
  size_t readBufferRead = 1;
  while (LOW != digitalRead(PN5180_BUSY)) {
    readBuffer[readBufferRead] = SPI.transfer(0xff);

    readBufferRead++;
  }

  // 4.
  digitalWrite(PN5180_NSS, HIGH); delay(1);

  // determine payload length
  size_t payloadLength;
  {
    size_t len1 = readBuffer[1] & (uint8_t)0x03;
    size_t len2 = readBuffer[2];

    payloadLength = (len1 << 8) | len2;
  }

  readBufferRead = payloadLength + 5;

  // determine crc16
  uint16_t payloadCrc16;
  {
    uint16_t c1 = readBuffer[3 + payloadLength];
    uint16_t c2 = readBuffer[4 + payloadLength];

    payloadCrc16 = (c1 << 8) | c2;
  }

  // verify crc16
  {
    auto r = phHal_Host_CalcCrc16(readBuffer + 1, payloadLength + 2);

    if (payloadCrc16 != r) {
      PN5180DEBUG(F("ERROR: CRC16 mismatch!\n"));
      return false;
    }
  }

  // copy data
  memcpy(recvBuffer, readBuffer + 3, std::min(recvBufferLen, readBufferRead));

  // debug output
#ifdef DEBUG
  PN5180DEBUG(F("Received: "));
  for (size_t i=0; i<readBufferRead; i++) {
    if (i > 0) PN5180DEBUG(" ");
    PN5180DEBUG(formatHex(readBuffer[i]));
  }
  PN5180DEBUG("'\n");
#endif

  return true;
}

void PN5180FirmwareUpdate::doUpdate() {
  // enable download mode
  Serial.println("Switching to download mode...");

  digitalWrite(PN5180_REQ, HIGH); // enable download mode
  digitalWrite(PN5180_RST, LOW);  // reset, at least 10us required
  delay(10);
  digitalWrite(PN5180_RST, HIGH); // 2ms to ramp up required
  delay(100);
  digitalWrite(PN5180_REQ, LOW);  // set back to low

  // get current version
  uint8_t vMajor, vMinor;

  getFirmwareVersion(vMajor, vMinor);

  Serial.print(F("Current PN5180 Firmware: v"));
  Serial.print(vMajor);
  Serial.print(".");
  Serial.println(vMinor);

  // check image
  const uint8_t *img = gphDnldNfc_DlSequence4_0;
  const size_t imgLen = gphDnldNfc_DlSeqSizeOf4_0;
  uint8_t imgMajor, imgMinor;

  if (!checkImage(img, imgLen, imgMajor, imgMinor))
    return;

  Serial.print(F("Image PN5180 Firmware: v"));
  Serial.print(imgMajor);
  Serial.print(".");
  Serial.println(imgMinor);

  // perform update
  uploadImage(img, imgLen);
}

PN5180FWState PN5180FirmwareUpdate::uploadImage(const uint8_t *img, size_t imgLen) {
  uint8_t *imgPtr = (uint8_t*)img;
  size_t imgRead = 0;
  uint8_t res[4];

  Serial.print("Updating.");

  while (imgRead < imgLen - 1) {
    // check command code
    if (imgPtr[2] != 0XC0) {
      Serial.println("FAILED");
      Serial.println("Found invalid command code in image!");
      return FWERR_MAGICNO;
    }

    // determine payload length
    size_t payloadLength;
    {
      size_t len1 = imgPtr[0] & (uint8_t)0x03;
      size_t len2 = imgPtr[1];

      payloadLength = (len1 << 8) | len2;
    }

    // send data
    SPI.beginTransaction(PN5180_SPI_SETTINGS);
    transceiveCommandDownloadMode(imgPtr, payloadLength + 2, res, sizeof(res));
    SPI.endTransaction();

    // check status
    if (res[0] != 0) {
      Serial.println("FAILED");
      Serial.print("Got error while uploading data: 0x");
      Serial.println(res[0], HEX);
      return (PN5180FWState)res[0];
    }

    // move forward
    imgPtr += payloadLength + 2;
    imgRead += payloadLength + 2;

    Serial.print(".");
  }

  Serial.println("done");

  return FWERR_OK;
}

bool PN5180FirmwareUpdate::checkImage(const uint8_t *img, size_t imgLen, uint8_t &major, uint8_t &minor) {
  if (imgLen < 1024) {
    Serial.println("Image size below 1 KB");
    return false;
  }

  // check for the signature
  static const uint8_t magic[] = { 0X00, 0XE4, 0XC0, 0X00 };

  if (memcmp(img, magic, sizeof(magic))) {
    Serial.println("Image magic number mismatch");
    return false;
  }

  // retrieve version
  major = img[5];
  minor = img[4];

  return true;
}

void PN5180FirmwareUpdate::getFirmwareVersion(uint8_t &major, uint8_t &minor) {
  uint8_t buf[] = { 0x00, 0x04, DOWNLOADCMD_GET_VERSION, 0x00, 0x00, 0x00 };
  uint8_t res[10];

  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  transceiveCommandDownloadMode(buf, sizeof(buf), res, sizeof(res));
  SPI.endTransaction();

  major = res[9];
  minor = res[8];
}

void PN5180FirmwareUpdate::end() {
  PN5180::end();

  reset();
}
