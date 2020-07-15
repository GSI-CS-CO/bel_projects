#include "ebwrapper.h"


Class EbDev public EbWrapper {

private:


  Socket ebs;
  Device ebd;

  std::vector<struct sdb_device> cpuDevs;
  std::vector<struct sdb_device> cluTimeDevs;
  std::vector<struct sdb_device> diagDevs;

public:
  EbDev(const std::string& ebdevname) : EbWrapper(ebdevname) {}
  ~EbDev()  {};
  bool connect(const std::string& en, bool test=false);
  bool disconnect(); //Close connection
  int writeCycle(const ebWrs& ew) const;
  int writeCycle(const vAdr& va, const vBuf& vb, const vBl& vcs)  const;
  int writeCycle(const vAdr& va, const vBuf& vb)  const;
  vBuf readCycle(const ebRds& er)  const;
  vBuf readCycle(const vAdr& va, const vBl& vcs) const;
  vBuf readCycle(const vAdr& va ) const;
  uint32_t read32b(uint32_t adr) const;
  uint64_t read64b(uint32_t startAdr) const;
  int write32b(uint32_t adr, uint32_t data) const;
  int write64b(uint32_t startAdr, uint64_t data) const;
  bool isSimulation()  const {return false;}
  void showCpuList() const;



};