#! /usr/bin/sh

# . modules/ftm/tests/tools/prepareRemoteSnoop.sh
uname -a
hostname
# cat modules/ftm/tests/tools/fel0069_known_host >> ~/.ssh/known_hosts
# cat ~/.ssh/known_hosts
# cat ~/.ssh/id\_ecdsa.pub
test -f ~/.ssh/id_ecdsa || ssh-keygen -t ecdsa -f ~/.ssh/id_ecdsa
ssh-copy-id -i ~/.ssh/id\_ecdsa.pub root@fel0069.acc.gsi.de
ssh root@fel0069.acc.gsi.de 'saft-ctl tr1 -xv snoop 0 0 0 1'
