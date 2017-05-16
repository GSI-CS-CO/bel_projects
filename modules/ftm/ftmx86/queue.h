 

class Queue : public Node {
  uint64_t  tPeriod;

public:

  Block(uint64_t tPeriod) : tPeriod(tPeriod) {}
  ~Block()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  uint32_t getAdr()     const  {return Node::getAdr();}
  void show(void)       const  {show(0, "");}
  void show(uint32_t cnt, const  char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)tPeriod);}
  void serialise(itBuf ib)  {printf("I'am a serialised Block\n");}
  

};

