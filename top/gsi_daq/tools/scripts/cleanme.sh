#!/bin/sh
###############################################################################
##                                                                           ##
##          Shellscript to remove possibly lost ssh logins                   ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:      cleanme.sh                                                     ##
## Author:    Ulrich Becker                                                  ##
## Date:      02.12.2021                                                     ##
## Copyright: GSI Helmholtz Centre for Heavy Ion Research GmbH               ##
###############################################################################


for i in $(ps -ef | grep $(id -un) | grep notty | awk '{print $2}')
do
   [ "$$" != "$i" ] || kill $i
done

#=================================== EOF ======================================
