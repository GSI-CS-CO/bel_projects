#!/bin/sh
# b2b deployment script: configure systemd  

logger "hacky unsupported software deployment script $0: start"

SERVICEA=b2b-servsys-intyrkickinj.service
SERVICEB=b2b-servraw-intyr.service
SERVICEC=b2b-analyzer-intyr.service

logger "$0: copying systemd service configurations"
cp -a /opt/$NAME/systemd/$SERVICEA /lib/systemd/system
cp -a /opt/$NAME/systemd/$SERVICEB /lib/systemd/system
cp -a /opt/$NAME/systemd/$SERVICEC /lib/systemd/system
systemctl daemon-reload

logger "$0: starting systemd services"
systemctl start $SERVICEA
systemctl start $SERVICEB
systemctl start $SERVICEC

logger "hacky unsupported software deployment script $0: done"
