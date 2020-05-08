import dm
from datetime import datetime
import pytz
 


dmobj = dm.CarpeDM()
dmobj.connect("dev/ttyUSB0")
dmobj.halt()
dmobj.clear(False)
dmobj.addDotFile("pps.dot", False);
status = dmobj.downloadDot(False)
result = dmobj.getAllPatterns()


print (result)
# blah blah lots of code ...




#dmobj.startPattern("PPS_TEST", 0)

wrt = dmobj.getDmWrTime()
d = datetime.fromtimestamp(int(wrt/1e9), pytz.timezone('Europe/Berlin'))

print("WR Time:  %s %u ns\n" % (d , wrt % 1e9 ))

print("CPU Message Counters:")
for i in range(0, 4):
	hr = dm.HealthReport()
	hr = dmobj.getHealth(i, hr)
	print("%s#: %u" % (i, hr.msgCnt))

