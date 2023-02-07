#!/bin/sh
# b2b deployment script: configure systemd  

logger "fire-up my latest shit $0: start"

SERVICEA=b2b-servsys-int-yr-kickinj.service
SERVICEB=b2b-servraw-int-yr.service
SERVICEC=b2b-analyzer-int-yr.service

logger "$0: copying systemd service configurations"
cp -a /opt/$NAME/systemd/$SERVICEA /lib/systemd/system
cp -a /opt/$NAME/systemd/$SERVICEB /lib/systemd/system
cp -a /opt/$NAME/systemd/$SERVICEC /lib/systemd/system
systemctl daemon-reload

logger "$0: starting systemd services"
systemctl start $SERVICEA
systemctl start $SERVICEB
systemctl start $SERVICEC

logger "fire-up my latest shit $0: done"
