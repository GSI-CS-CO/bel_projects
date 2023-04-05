#! /usr/bin/sh

test -f ~/.ssh/id_ecdsa || ssh-keygen -t ecdsa -f ~/.ssh/id_ecdsa
ssh-copy-id -i ~/.ssh/id\_ecdsa.pub root@fel0069.acc.gsi.de
ssh root@fel0069.acc.gsi.de 'saft-ctl tr1 -xv snoop 0 0 0 1'
