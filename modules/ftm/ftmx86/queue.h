

class Queue : public Node {
  uint64_t  period;

public:

  TimeBlock(uint64_t period) : period(period) {}
  ~TimeBlock()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  uint32_t getAdr()     const {return Node::getAdr();}
  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itBuf ib)  {printf("I'am a serialised Timeblock\n");}
  

};

