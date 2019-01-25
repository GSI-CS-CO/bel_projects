#DM Test Manager v 0.1
import difflib
import sys
import os.path
import subprocess
import time

prefix ="/usr/local/bin/dm-"
diffHeaderLen = 2


class Filter(object):
    # Create based on class name:
    def factory(type):
        #return eval(type + "()")
        if type is not None:
          if type == "reltime": 
            #print("creating timefilert")
            return Timefilter()
          if type == "msgcnt": 
            #print("creating timefilert")
            return Msgcntfilter()  
          assert 0, "Bad filter creation: " + type
    factory = staticmethod(factory)

class Timefilter:
  
  #  self.lambdas = lambdas
  #  self.paras = parameters
#
  #def run:
  def run(self, diff):
    toRemove = []
    for line in diff:
      if "VABS:0" in line:
        #could check against actual time
        toRemove.append(line)
    for line in toRemove:
      diff.remove(line)        
    return diff

class Msgcntfilter:
  
  #  self.lambdas = lambdas
  #  self.paras = parameters
#
  #def run:
  def run(self, diff):
    toRemove = []
    for line in diff:
      if "MSG:" in line:
        #could check against actual time
        toRemove.append(line)
    for line in toRemove:
      diff.remove(line)        
    return diff           
      

class Op:
  def __init__(self, desc, typ, content, execTime=0.0, optFile=None, expResFile=None, filtertype=None):
    self.desc       = desc
    self.typ        = typ
    self.content    = content
    self.execTime   = float(execTime)
    self.optFile    = optFile
    self.expResFile = expResFile
    self.__filter     = Filter.factory(filtertype)
    self.expRes     = None
    self.path       = None
    self.result     = None 
    self.passed     = None   
  
  def runFilter(self, diff):
    if self.__filter is not None:
      ##print("op %s has a filter" % (self.desc))
      return self.__filter.run(diff)
    return diff    

  def setPath(self, path):
    self.path = path


  def getOptFileName(self):
    if self.optFile is not None:
      return self.path + "/" + self.optFile
    return ""    

  def getCliStr(self, dev):
    execStr = prefix + self.typ + " " + dev + " " + self.content + " " + self.getOptFileName();
    return execStr

  def getCliCmd(self):
    execStr = prefix + self.typ
    return execStr
    
  def getCliArgs(self, dev):
    execList = [dev]
    execList += self.content.split()
    execList.append(self.getOptFileName())
    return execList
    #execStr = " " + dev + " " + self.content
    #if self.optFile is not None:
    #  execStr += " " + self.path + self.optFile
    #return execStr      

  def getExpRes(self):
    if self.expRes is None and self.expResFile is not None:
      with open(self.path + "/" + self.expResFile) as f:
        self.expRes = f.read().splitlines() 
    return self.expRes

  def runEb(self, dev):
    res = ""
    optFile = ""
    if self.optFile is not None:
      optFile = self.path + self.optFile
    try:
      time.sleep(float(self.execTime))   
      tmp = subprocess.check_output([self.getCliCmd()] + self.getCliArgs(dev), stderr=subprocess.STDOUT)
      if self.expResFile is not None:
        d='\n'
        self.result = tmp.decode("utf-8", "ignore").split(d) # [e+d for e in tmp.decode("utf-8", "ignore").split(d) if e]
        self.result.pop() # remove trailing newline
    except subprocess.CalledProcessError as err:
      print("Rcode %s Stdout %s" % (err.returncode, err.output)) 

      
       
    

  def runRda(self, instance):
    pass  

  def showRes(self):
    if self.result is not None:
      print(self.result)

  def getOutcome(self):
    return self.passed
            



  def diff(self):
    sret = []
    expRes = self.getExpRes()
    if expRes is not None and self.result is not None:
      #print(expRes)
      #print(self.result)
      sret.append("# DIFF " + self.desc)
      sret.append("# - " + self.path + "/" + self.expResFile + "\n# + test result\n")
      d = difflib.Differ()
      diffs = list(d.compare(expRes, self.result))
      #print(diffs)
      lineNum = 0
      for line in diffs:
        
        # split off the code
        code = line[:2]
        # if the  line is in both files or just b, increment the line number.
        if code in ("  ", "- "):
           lineNum += 1
        # if this line is only in b, print the line number and the text on the line
        if code == "- " or code == "+ ":
          sret.append("%04d: %s" % (lineNum, line))
    return sret      

class TestCase:
  def __init__(self, name, opList):
    self.name = name
    self.path = None
    self.opList = opList
    self.result = None

  def showOpsList(self):
    for op in self.opList:
      optFile = "-"
      if op.optFile is not None:
        optFile = self.path + "/" +  op.optFile  
      expResFile = "-"
      if op.expResFile is not None:
        expResFile = self.path + "/" +  op.expResFile  
      print("# %s, '%s', %fs, %s %s" % (op.desc, op.getCliStr(self.dev), op.execTime, optFile, expResFile))
    print("")  

  def setPath(self, filename):
    if os.path.isfile(filename):
      mypath, myfile = os.path.split(filename)        
      if not mypath:
        mypath += './'
      self.path = mypath
      for op in self.opList:
        op.setPath(self.path)
    else:
      print("Manifest contains invalid path in <%s>" % filename)

  def setDev(self, dev):
    self.dev = dev

  def runOp(self, index, mode="eb"):
    if mode == "eb":
      self.opList[index].runEb(self.dev)
    else:
      self.opList[index].runRda(self.rda)  
    self.opList[index].showRes()

  def run(self, mode="eb"):
    for op in self.opList:
      if mode == "eb":
        op.runEb(self.dev)
      else:
        op.runRda(self.rda)

  def show(self):
    for op in self.opList:
      op.showRes()      

  

  def eval(self, f=False, index=-1):
    failed = False
    diff = []
    if index == -1:
      for op in self.opList:
        tmpDiff = op.diff()
        if not tmpDiff:
          diff.append([])
          continue 
        
        if f == True:
          #print ("len tmpDiff b4 %u" % len(tmpDiff))
          tmpDiff = op.runFilter(tmpDiff)
          #print ("len tmpDiff af %u" % len(tmpDiff)) 
        if len(tmpDiff) > diffHeaderLen:
          #print ("%s failed" % self.name)
          op.passed = False
        else:
          #print ("%s passed" % self.name) 
          op.passed = True
        diff.append(tmpDiff)
        
    else:
      if not tmpDiff:
        self.opList[index].passed = True
        pass
      tmpDiff = self.opList[index].diff()
      #print ("len tmpDiff b4 %u" % len(tmpDiff))
      if f == True:
        tmpDiff = self.opList[index].runFilter(tmpDiff)
        #print ("len tmpDiff af %u" % len(tmpDiff))
      diff.append(tmpDiff)
    return diff  

  def report(self, diff, index=-1):
    passedCnt = 0
    failedCnt = 0
    nocmpCnt = 0
    ret = False
    #print (diff)
    for index, op in enumerate(self.opList):  
      #print ("op out %s" % op.getOutcome())  
      if op.getOutcome() is not None:
        if op.getOutcome() == True:
          #print("PASSED\n")
          passedCnt += 1
        else:
          test = diff[index]
          for line in test:
            print(line)
          print("FAILED\n")
          failedCnt += 1
      else:
        nocmpCnt += 1
    if failedCnt == 0:
      print("Test <%s> PASSED" % self.name)
      ret = True
    else:
      print("Test <%s> FAILED" % self.name)
      ret = False
    print("Result of Operations:\n%u no expectations, %u passed, %u failed\n%u total" %(nocmpCnt, passedCnt, failedCnt, int(nocmpCnt) + int(passedCnt) + int(failedCnt)))
    return ret

        
 # def runOp    


class Manager:
  def __init__(self, basepath, manifestFile, dev):
    self.basepath = basepath
    exec(open(basepath + "/" + manifestFile).read(), locals())
    self.tests        = locals()["testfiles"]
    self.dev          = dev
    self.actTestCase  = None #TestCase("hallo", [Op("Init0", "cmd", "halt"),])
    self.actResult    = []



  def showTestList(self):
    print(self.tests)

  def loadTest(self, index):
    if index < len(self.tests):
      testrelative = self.tests[index].split("/")
      # if first result is alphanumeric, we can directly concatenate
      if "." in testrelative[0]:
        fulltestname = self.basepath + "/" + self.tests[index][2:]
      else:  
        fulltestname = self.basepath + "/" + self.tests[index]
      exec(open(fulltestname).read(), globals(), locals())
      self.actTestCase = locals()["testcase"]
      self.actTestCase.setPath(fulltestname)
      self.actTestCase.setDev(self.dev)
    else:
      print("Index not in list of tests")   

  #def runTest(self):
  #  if self.actTestCase is not None:


def main(argv):
  pathandfile = os.path.realpath(__file__)
  mypath, myfile = os.path.split(pathandfile) 
  #print (mypath)
  m = Manager(mypath, "TestManifest.py", "tcp/tsl008.acc")
  print("Loading %u Tests" % len(m.tests))
  #m.showTestList()
  print("\n########################\n")
  failedList = []
  for index, test in enumerate(m.tests):
    m.loadTest(index)
    #m.actTestCase.showOpsList()
    m.actTestCase.run()
    if m.actTestCase.report(m.actTestCase.eval(True)) == False:
      failedList.append(index)
  print("\n########################\n")  
  if len(failedList) == 0:
    print("Test run PASSED")

  else:
    print("Test run FAILED")
    for test in failedList:
      print("Test <%s> FAILED" % m.tests[test])


# print("Result of Tests:\n%u passed, %u failed\n%u total" %(len(m.tests) - len(failedList), len(failedList), len(m.tests)))
  

if __name__ == "__main__":
  main(sys.argv[1:])


