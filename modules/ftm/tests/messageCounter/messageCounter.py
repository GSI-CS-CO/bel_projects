import dm
from datetime import datetime
import pytz
import sys

device = sys.argv[1]
dmobj = dm.CarpeDM()
dmobj.connect(device)
wrt = dmobj.getDmWrTime()
d = datetime.fromtimestamp(int(wrt/1e9), pytz.timezone('Europe/Berlin'))

print("WR Time:  %s %u ns\n" % (d , wrt % 1e9 ))

print("%s: CPU Message Counters:" % device)
for i in range(0, 4):
	hr = dm.HealthReport()
	hr = dmobj.getHealth(i, hr)
	print("%s#: %u" % (i, hr.msgCnt))
