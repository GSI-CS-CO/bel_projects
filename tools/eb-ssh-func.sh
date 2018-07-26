function _add_eb_aliases()
{
  alias eb-ls='_eb_ls'
  alias eb-info='_eb_info'
  alias eb-read='_eb_read'
  alias eb-write='_eb_write'
  alias eb-get='_eb_get'
  alias eb-put='_eb_put'
  alias eb-mon='_eb_mon'
  alias eb-fwload='_eb_fwload'
  alias eb-find='_eb_find'
  alias eb-reset='_eb_reset'
  alias eb-console='_eb_console'
}

function _add_dm_aliases()
{
  alias dm-cmd='_dm_cmd'
  alias dm-sched='_dm_sched'
}

function _rm_eb_aliases()
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

function _rm_dm_aliases()
{
  unalias dm-cmd
  unalias dm-sched
}



function _eb_func() {
  local rawCmd pciDev dev sshDev sshAuxCmd sshCmd directCmd
  rawCmd=$@                                                             # input from caller func
  pciDev="dev/wbm0"                                                     # constant for pci device string
  dev=`echo $rawCmd | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`  # isolate device string from input
  sshDev=`echo $dev | cut -d\/ -f2`                                     # remove tcp/ udp/ dev/ from device string for ssh hostname
  sshAuxCmd="${rawCmd/$dev/$pciDev}"                                    # replace device string with pci device string for ssh access
  sshCmd="ssh root@$sshDev $sshAuxCmd"                                  # the full ssh command
  directCmd="command $rawCmd"                                           # the full direct command, bypass alias to avoid loop

  if [[ $dev = *"tcp/"* ]]; then
    #echo "ssh"
    #echo "$sshCmd";
    eval $sshCmd
  else
    #echo "direct"
    #echo "$directCmd";
    eval $directCmd
  fi
}

function _eb_ls() {
  _eb_func "eb-ls $@"
}

function _eb_info() {
  _eb_func "eb-info $@"
}

function _eb_read() {
  _eb_func "eb-read $@"
}

function _eb_write() {
  _eb_func "eb-write $@"
}

function _eb_get() {
  _eb_func "eb-get $@"
}

function _eb_put() {
  _eb_func "eb-put $@"
}

function _eb_mon() {
  _eb_func "eb-mon $@"
}

function _eb_fwload() {
  _eb_func "eb-fwload $@"
}

function _eb_find() {
  _eb_func "eb-find $@"
}

function _eb_reset() {
  _eb_func "eb-reset $@"
}

function _eb_console() {
  _eb_func "eb-console $@"
}


# DM tools
function _dm_sched() {
  _eb_func "dm-sched $@"
}

function _dm_cmd() {
  _eb_func "dm-cmd $@"
}