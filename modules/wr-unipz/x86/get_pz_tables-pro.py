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

d = devacc.Device("GUR_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GUL_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GUN2ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GUN_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GUH_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GUA_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
d = devacc.Device("GTK_ZC")
for vacc in range(0, 16) :
    devacc.vrtacc.set(vacc)
    for channel in range(0, 2):
        r = d.read("EVTSEQU", channel)
        print r
        

print ''
print 'next time, run with "> filename"'



