#! /usr/bin/python2                                                                                        
# -*- coding: utf-8 -*-
 
"""
created on 2019-January-28
@author: Dietrich Beck, GSI

my first python program ...

this dumps the event tables of UNIPZ

try to run this on the asl6 cluster
"""

import devacc

d = devacc.Device("GU_TST_Z1")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z2")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z3")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z4")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z5")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z6")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GU_TST_Z7")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
        

print ''
print 'next time, run with "> filename"'



