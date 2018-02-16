import datetime
import os.path
import sys
import getopt
import shutil
import re
from collections import namedtuple

ActStruct = namedtuple('ActStruct', ['ex', 'graph', 'wait'])

  

def parseLogFile(logFileIn):
  regpat_action = re.compile( '[a-z]+(?=([: *]\sdigraph))', re.VERBOSE | re.MULTILINE)
  regpat_graph  = re.compile( '(digraph)([\S\s]*?\})(?=(\s+Generator))'  , re.VERBOSE | re.MULTILINE)


  buffer = open(logFileIn).read()

  e = []
  g = []
  for match in regpat_action.finditer(buffer):
    e.append(match.group())
  for match in regpat_graph.finditer(buffer):
    g.append(match.group())  



  ret = []  
  for thisExec, thisGraph in zip(e, g):
    #print "%s" % (thisExec)
    #print "%s" % (thisGraph)
    myAs = ActStruct(ex=thisExec , graph=thisGraph , wait='0.0')  
    ret.append(myAs)

  return ret

def writeOutCmdLine(dev, actions, outdir, outfilenbasename, includeCmdDots):
  dev = "$1"
  outscript = "%s/%s_cmdl.sh" % (outdir, outfilenbasename)
  print "Writing to %s" % (outscript)
  fso = open(outscript, "w")
  fso.write("#!/bin/bash\n")

  cnt = 0
  for act in actions:
    outfilename = "%s%04d.dot" % (outfilenbasename, cnt)
    
    if act.ex == "command":
      if includeCmdDots == True:
        shcmdaux  = "dm-cmd %s -i %s" % (dev, outfilename)
        shcmd     = "echo %s; %s" % (shcmdaux, shcmdaux)
      else:
        shcmd = "#Skipping Cmd Dot %s" % (outfilename) 
    else:
      shcmdaux  = "dm-sched %s %s %s" % (dev, act.ex, outfilename)
      shcmd     = "echo %s; %s" % (shcmdaux, shcmdaux)
    shwait = "sleep %s" % (act.wait)
    
    outfile = "%s/%s" % (outdir, outfilename)
    
    
    fso.write("%s; %s #%03d\n" % (shcmd, shwait, cnt))
    print "Writing to %s" % outfile
    fgo = open(outfile, "w")
    for line in act.graph:
      fgo.write(line)
    fgo.close()

    cnt += 1
  
  fso.close




def writeOutFesa(server, dev, actions, outdir, outfilenbasename, includeCmdDots):
  sRda  = "/common/usr/cscosv/bin/rda-"
  sInit = "#!/bin/bash\nexport CMW_DIRECTORY_CLIENT_SERVERLIST=\"%s\"" % server
  sSchedClear   = sRda + "set -d GeneratorDeviceInstance -p ScheduleClear -v 1"
  sCatAux       = "dot=`printf \"%%s\" \"\`cat %s\`\"`\n"
  sSchedUpload  = sCatAux + sRda + "set -d GeneratorDeviceInstance -p UploadSchedule -f uploadSchedule -t string -v \"$dot\""
  sCmdUpload    = sCatAux + sRda + "set -d GeneratorDeviceInstance -p DirectorCommands -f directorCommands -t string -v \"$dot\""
  sCmdExec      = sRda + "set -d GeneratorDeviceInstance -p DirectorCommandsExecute -t int8 -v 1"
  sSchedAdd     = sRda + "set -d GeneratorDeviceInstance -p ScheduleAdd -t int8 -v 1"
  sSchedKeep    = sRda + "set -d GeneratorDeviceInstance -p ScheduleKeep -t int8 -v 1"


  outscript = "%s/%s_fesa.sh" % (outdir, outfilenbasename)
  print "Writing to %s" % (outscript)
  fso = open(outscript, "w")
  fso.write("%s\n" % sInit)
  fso.write("%s\n" % sSchedClear)

  cnt = 0
  for act in actions:
    outfilename = "%s%04d.dot" % (outfilenbasename, cnt)
    
    if act.ex == "command":
      if includeCmdDots == True:
        shcmdaux0  = sCmdUpload % (outfilename)
        shcmdaux1  = "%s" % (sCmdExec)
        shcmd     = "echo %s\n%s\n%s" % (shcmdaux0, shcmdaux0, shcmdaux1)
      else:
        shcmd = "#Skipping Cmd Dot %s" % (outfilename) 
    else:
      shcmdaux0  = sSchedUpload % (outfilename)
      if act.ex == "add":
        shcmdaux1  = "%s" % (sSchedAdd)
      elif act.ex == "keep":  
        shcmdaux1  = "%s" % (sSchedKeep)
      shcmd     = "echo %s\n%s\n%s" % (shcmdaux1, shcmdaux0, shcmdaux1)
    shwait = "sleep %s" % (act.wait)
    
    outfile = "%s/%s" % (outdir, outfilename)
    
    
    fso.write("%s; %s #%03d\n" % (shcmd, shwait, cnt))
    

    cnt += 1
  
  fso.close




def main():

    def version():
        for line in s.versionText:        
            print line 
                    
      
    if(len(sys.argv) > 3):
        ebdev = sys.argv[1]
        logFileIn = sys.argv[2]
        folderName = sys.argv[3]
    else:
        print "Usage: mirror_blast <ebdev> <logfile> <outdir> -f (force overwrite)"
        sys.exit(2)     
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hf", ["help", "force"])
    except getopt.GetoptError, err:
        # print help information and exit:
        print str(err) # will print something like "option -a not recognized"
        sys.exit(2)    
    

    force    = False
    needFile = True
    for option, argument in opts:
        if option in ("-h", "-?", "--help"):
            needFile = False
            manual()
        elif option in ("-f", "--force"):
            force = True    
        else:
            print "unhandled option %s" % option
            sys.exit(2) 
    
    
    if(needFile):
      
        

        
        

        if os.path.isfile(logFileIn):
            mypath, myfile = os.path.split(logFileIn)        
            if not mypath:
                mypath += './'
            
            

            #Create output directory at <inputpath/foldername>
            outpath = mypath + folderName
            print "Looking for folder <%s>" % (outpath)
            if os.path.exists(outpath):
              print "directory <%s> already exists" % (outpath)
              if force:
                print "forced to erase"
                shutil.rmtree(outpath)
                os.makedirs(outpath)
              else:  
                sys.exit(1)
            else:
              print "Creating <%s>" % (outpath)  
              os.makedirs(outpath)
            if not os.path.exists(outpath):
              print "Error creating directory <%s>" % (outpath)  
            

            #Create project directory  
            now = datetime.datetime.now()

          


            print "f: %s p: %s" % (myfile, mypath)
            
                
            print "input/output dir: %s" % mypath
            print "Trying to parse:  %s" % myfile
            print "\n%s" % ('*' * 80)
                        
            actions = parseLogFile(logFileIn)
            writeOutCmdLine('$1', actions, outpath, folderName, False)
            writeOutFesa('cmwdev00a.acc.gsi.de:5021', '$1', actions, outpath, folderName, False)
            
            print "\n%s" % ('*' * 80) 
            print "\nDone!"
        else:
            print "\nFile not found: %s" % xmlIn
    
    


if __name__ == "__main__":
    main()

