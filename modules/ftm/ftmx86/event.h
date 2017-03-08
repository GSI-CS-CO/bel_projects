#ifndef _EVENT_H_
#define _EVENT_H_


#include <stdlib.h>
#include <stdint.h>

class Event {
  uint64_t tOffs;

public:
  Event();
  Event(uint64_t tOffs, uint16_t flags);
  virtual ~Event();
  virtual void show();
  virtual uint8_t* serialise(uint8_t *buf);

};

class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;

public:
  TimingMsg(uint64_t tOffs, uint16_t flags, uint64_t id, uint64_t par, uint32_t tef) : Event (tOffs, flags);
  ~TimingMsg();
  void show();
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *buf);
};

class Command : public Event {
  uint64_t tValid;
  uint32_t act;

public:
  Command(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint32_t act) : Event (tOffs, flags);
  ~Command();
  void show();
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *buf);
};



#endif
