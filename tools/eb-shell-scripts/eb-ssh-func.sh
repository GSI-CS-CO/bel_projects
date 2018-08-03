#!/bin/bash
# EB SSH Wrapper Functions
# 2018 by Mathias Kreider, GSI
#
# Automatically translates eb-tool actions over TCP via socat into SSH commands to the host platform
# To override this and use original eb-tools behaviour for 'tcp/' devices,
# prefix the call with a backslash, eg '\eb-ls tcp/...'
#
# CAVEAT: While other syntaxes with preceding 'command' or absolute paths also work for tcp/SSH override,
# they will cause trouble when using autocompletion for dm-cmd. The reason is that autocomplete only reads
# the command word (dm-cmd) and onward, all preceding words are lost. We can thus only make the background call
# to dm-sched also use the override if we got told as part of the dm-cmd word, i.e. '\dm-cmd'
# Best to only ever use '\<eb-tool> tcp/<eb-device>' syntax when overriding.
#
#
# Install:
# this file belongs in your home directory and should be sourced by .bashrc
#
# cat append_eb_ssh_to_bashrc >> ~/.bashrc
# source ~/.bashrc
#
# You can check if the automation is installed by typing:
# alias
#
# As a real world test, deactivate socat on frontend, try: '\eb-ls tcp/<front-end>'. This should fail.
# Now call the aliased version (the usual call):            'eb-ls tcp/<front-end>'. This should work.

_add_eb_aliases()
{
  alias eb-ls='_eb_ssh_eb_ls'
  alias eb-info='_eb_ssh_eb_info'
  alias eb-read='_eb_ssh_eb_read'
  alias eb-write='_eb_ssh_eb_write'
  alias eb-get='_eb_ssh_eb_get'
  alias eb-put='_eb_ssh_eb_put'
  alias eb-mon='_eb_ssh_eb_mon'
  alias eb-fwload='_eb_ssh_eb_fwload'
  alias eb-find='_eb_ssh_eb_find'
  alias eb-reset='_eb_ssh_eb_reset'
  alias eb-console='_eb_ssh_eb_console'
}

_add_dm_aliases()
{
  alias dm-cmd='_eb_ssh_dm_cmd'
  alias dm-sched='_eb_ssh_dm_sched'
}

_rm_eb_aliases()
{
  unalias eb-ls
  unalias eb-info
  unalias eb-read
  unalias eb-write
  unalias eb-get
  unalias eb-put
  unalias eb-mon
  unalias eb-fwload
  unalias eb-find
  unalias eb-reset
  unalias eb-console
}

_rm_dm_aliases()
{
  unalias dm-cmd
  unalias dm-sched
}

#use _eb_ssh_ prefix as a sort of namespace to clashes with possible autocomplete functions

_eb_ssh_eb_func() {
  local rawCmd pciDev dev sshDev sshAuxCmd sshCmd directCmd fileWithPath fileNoPath
  rawCmd=$@                                                             # input from caller func
  pciDev="dev/wbm0"                                                     # constant for pci device string
  dev=`echo $rawCmd | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`  # isolate device string from input
  fileWithPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-\/]*.(bin|dot|rpd)"`  # isolate qualified file string from input
  fileNoPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-]*.(bin|dot|rpd)"`      # isolate file string from input

  sshDev=`echo $dev | cut -d\/ -f2`                                     # remove tcp/ udp/ dev/ from device string for ssh hostname
  sshAuxCmd="${rawCmd/$dev/$pciDev}"                                    # replace device string with pci device string for ssh access
  sshAuxCmd="${sshAuxCmd/$fileWithPath/$fileNoPath}"                    # remove path from file names
  sshCmd="ssh -t root@$sshDev \"cd /tmp; $sshAuxCmd\""                  # the full ssh command
  directCmd="command $rawCmd"                                           # the full direct command, bypass alias to avoid loop

  if [[ $dev = *"tcp/"* ]]; then
    eval $sshCmd
  else
    eval $directCmd
  fi
}

_eb_ssh_eb_cpy_file_to_fe() {
  local rawCmd dev sshDev scpCmd fileWithPath
  rawCmd=$@                                                             # input from caller func
  dev=`echo $rawCmd | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`  # isolate device string from input
  fileWithPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-\/]*.(bin|dot|rpd)"`  # isolate file string from input
  sshDev=`echo $dev | cut -d\/ -f2`                                     # remove tcp/ udp/ dev/ from device string for ssh hostname
  scpCmd="scp $fileWithPath root@$sshDev:/tmp/"                            # secure copy the file from caller pc to frontend


  if [[ $dev = *"tcp/"* ]] && [[ -n $fileWithPath ]]; then
    eval $scpCmd
  fi
}

_eb_ssh_eb_cpy_file_from_fe() {
  local rawCmd dev sshDev scpCmd fileWithPath fileNoPath
  rawCmd=$@                                                                     # input from caller func
  dev=`echo $rawCmd | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`          # isolate device string from input
  fileWithPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-\/]*.(bin|dot|rpd)"`   # isolate qualified file string from input
  fileNoPath=`echo $rawCmd | grep -o -E "[a-zA-Z0-9\_\-]*.(bin|dot|rpd)"`       # isolate file string from input
  sshDev=`echo $dev | cut -d\/ -f2`                                             # remove tcp/ udp/ dev/ from device string for ssh hostname
  scpCmd="scp root@$sshDev:/tmp/$fileNoPath $fileWithPath"                      # secure copy the file from frontend to caller pc


  if [[ $dev = *"tcp/"* ]] && [[ -n $fileWithPath ]]; then
    eval $scpCmd
  fi
}

_eb_ssh_eb_cpy_download_dot_from_fe() {
  local rawCmd dev sshDev scpCmd
  rawCmd=$@                                                                     # input from caller func
  dev=`echo $rawCmd | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`          # isolate device string from input
  sshDev=`echo $dev | cut -d\/ -f2`                                             # remove tcp/ udp/ dev/ from device string for ssh hostname
  scpCmd="scp root@$sshDev:/tmp/download.dot ."                                 # secure copy the file from frontend to caller pc


  if [[ $dev = *"tcp/"* ]]; then
    eval $scpCmd
  fi
}

_eb_ssh_eb_ls() {
  _eb_ssh_eb_func "eb-ls $@"
}

_eb_ssh_eb_info() {
  _eb_ssh_eb_func "eb-info $@"
}

_eb_ssh_eb_read() {
  _eb_ssh_eb_func "eb-read $@"
}

_eb_ssh_eb_write() {
  _eb_ssh_eb_func "eb-write $@"
}

_eb_ssh_eb_get() {
  _eb_ssh_eb_func "eb-get $@"
  _eb_ssh_eb_cpy_file_from_fe "eb-get $@"
}

_eb_ssh_eb_put() {
  _eb_ssh_eb_cpy_file_to_fe "eb-put $@"
  _eb_ssh_eb_func "eb-put $@"
}

_eb_ssh_eb_mon() {
  _eb_ssh_eb_func "eb-mon $@"
}

_eb_ssh_eb_fwload() {
  _eb_ssh_eb_cpy_file_to_fe "eb-fwload $@"
  _eb_ssh_eb_func "eb-fwload $@"
}

_eb_ssh_eb_find() {
  _eb_ssh_eb_func "eb-find $@"
}

_eb_ssh_eb_reset() {
  _eb_ssh_eb_func "eb-reset $@"
}

_eb_ssh_eb_console() {
  _eb_ssh_eb_func "eb-console $@"
}


# DM tools
_eb_ssh_dm_sched() {
  _eb_ssh_eb_cpy_file_to_fe "dm-sched $@"
  _eb_ssh_eb_func "dm-sched $@"
  _eb_ssh_eb_cpy_download_dot_from_fe "dm-sched $@"
}

_eb_ssh_dm_cmd() {
  _eb_ssh_eb_cpy_file_to_fe "dm-cmd $@"
  _eb_ssh_eb_func "dm-cmd $@"
}
