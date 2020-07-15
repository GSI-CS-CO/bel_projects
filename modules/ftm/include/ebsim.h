#include "ebwrapper.h"


Class EbSim public EbWrapper {

private:

 
  std::vector<uint32_t *> simRam;
  std::map<uint8_t, uint32_t> simRamAdrMap;
 
  void simAdrTranslation (uint32_t a, uint8_t& cpu, uint32_t& arIdx);
  void simRamWrite (uint32_t a, eb_data_t d);
  void simRamRead (uint32_t a, eb_data_t* d);

public:
  EbSim(const std::string& ebdevname) : EbWrapper(ebdevname) {}
  ~EbSim()  {};
  bool connect();
  bool disconnect(); //Close connection
  int writeCycle(const ebWrs& ew);
  int writeCycle(vAdr va, vBuf& vb, vBl vcs);
  int writeCycle(vAdr va, vBuf& vb);
  vBuf readCycle(const ebRds& er);
  vBuf readCycle(vAdr va, vBl vcs);
  vBuf readCycle(vAdr va );
  uint32_t read32b(uint32_t adr);
  uint64_t read64b(uint32_t startAdr);
  int write32b(uint32_t adr, uint32_t data);
  int write64b(uint32_t startAdr, uint64_t data);
  bool isSimulation()  const {return true;}  
  void showCpuList() const;  



};