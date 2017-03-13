#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "ftm_common.h"

static uint8_t* uint16ToBytes(uint8_t* pBuf, uint16_t val) {
  uint8_t i;
  for(i=0;i<2;   i++) pBuf[i]  = val >> (8*i) & 0xff;
  return pBuf+2;
}

static uint16_t bytesToUint16(uint8_t* pBuf) {
  uint8_t i;
  uint32_t val=0;
  for(i=0;i<2;   i++) val |= (uint16_t)pBuf[i] << (8*i);
  return val;
}


static uint8_t* uint32ToBytes(uint8_t* pBuf, uint32_t val) {
  uint8_t i;
  for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*i) & 0xff;
  return pBuf+4;
}

static uint32_t bytesToUint32(uint8_t* pBuf) {
  uint8_t i;
  uint32_t val=0;
  for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*i);
  return val;
}

static uint8_t* uint64ToBytes(uint8_t* pBuf, uint64_t val) {
  uint32ToBytes(pBuf+0, (uint32_t)(val>>32));
  uint32ToBytes(pBuf+4, (uint32_t)val);
  return pBuf+8;
}

static uint64_t bytesToUint64(uint8_t* pBuf) {
  uint64_t val=0;
  val |= (uint64_t)bytesToUint32(pBuf+0)<<32;
  val |= (uint64_t)bytesToUint32(pBuf+4);
  return val;
}


uint8_t* Event::serialise(uint8_t* pBuf) {
  
  uint64ToBytes(pBuf + EVT_OFFS_TIME,   this->tOffs);
  uint16ToBytes(pBuf + EVT_FLAGS,       this->flags);
  
  return pBuf;

}

uint8_t* TimingMsg::serialise(uint8_t* pBuf, uint32_t &freeBytes) {
  uint8_t *pC = (uint8_t*)pBuf;  
  if (pC != NULL && freeBytes >= _EVT_SIZE) {
    Event::serialise(pC); //call protected base serialiser
    uint16ToBytes(pC + EVT_TYPE, EVT_TYPE_TMSG);
    pC += _EVT_HDR_SIZE;
    uint64ToBytes(pC + EVT_TM_ID,  this->id);
    uint64ToBytes(pC + EVT_TM_PAR, this->par);
    uint32ToBytes(pC + EVT_TM_TEF, this->tef);
    pC        += _EVT_SIZE;
    freeBytes -= _EVT_SIZE;  
  }
  return pC;
}

uint8_t* Command::serialise(uint8_t* pBuf, uint32_t &freeBytes) {
  uint8_t *pC = (uint8_t*)pBuf;  
  if (pC != NULL && freeBytes >= _EVT_SIZE) {
    Event::serialise(pC); //call protected base serialiser
    uint16ToBytes(pC + EVT_TYPE, EVT_TYPE_CMD);
    pC += _EVT_HDR_SIZE;
    uint64ToBytes(pC + EVT_CM_TIME,  this->tValid);
    uint64ToBytes(pC + EVT_CM_ACT, this->act); // this is probably more complicated if action is a class ...
    uint32ToBytes(pC + EVT_CMD_RESERVED, 0); //pad
    pC        += _EVT_SIZE;
    freeBytes -= _EVT_SIZE;  
  }
  return pC;
}


void TimingMsg::show(void) {
  TimingMsg::show(0, "");
}

void TimingMsg::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu\n", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}

void Command::show(void) {
  Command::show(0, "");
}

void Command::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu\n", p, (long long unsigned int)this->tOffs);
  printf("%sValid @ %llu, Action 0x%08x\n", p, (long long unsigned int)this->tValid, this->act);
}






