#!/bin/sh
# dmunipz deployment script: configure systemd  

logger "fire-up my latest shit $0: start"

SERVICEA=dmunipz-logger.service

logger "$0: copying systemd service configurations"
cp -a /opt/$NAME/systemd/$SERVICEA /lib/systemd/system
systemctl daemon-reload

logger "$0: starting systemd services"
systemctl start $SERVICEA

logger "fire-up my latest shit $0: done"
