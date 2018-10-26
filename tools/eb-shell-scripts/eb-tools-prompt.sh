#!/bin/bash

# Autocomplete for all EB tools
#
# Install:
# This file belongs in /etc/bash_completion.d/
# After copying the file, you need to either restart the shell or source the file:
# . /etc/bash_completion.d/eb-tools-prompt.sh
#

_get_eb_dev()
{
	local dm_devlist
	dm_devlist=`history | cut -c 8- | grep -v "grep" | grep "\(eb\-\)[a-zA-Z0-9\-\_]*" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`

	echo $dm_devlist
}
 _eb_tools()
{
    local cur devlist dev
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    dev=`echo "$COMP_LINE" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`

    if [ -z $dev ]; then
    	devlist=$(_get_eb_dev);
    	COMPREPLY=( $(compgen -W "${devlist}" -- $cur) )
   	fi

    return 0

}

 _eb_flash()
{
    local cur devlist dev file
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
	dev=`echo "$COMP_LINE" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`
	file=`echo "$COMP_LINE" | grep -o -E "!*.rpd"`

    if [ -z $dev ]; then
    	devlist=$(_get_eb_dev);
    	COMPREPLY=( $(compgen -W "${devlist}" -- $cur) )
   	elif [ -z $file ]; then
    	COMPREPLY=( $( compgen -f -X '!*.rpd' -- $cur ) )
   	fi

    return 0

}

 _eb_fwload()
{

    local cur devlist dev file cpu offset
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    dev=`echo "$COMP_LINE" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`
	file=`echo "$COMP_LINE" | grep -o -E "!*.bin"`
    cpu=`echo "$COMP_LINE" | grep -o -E " (u[0-9]+|w|) "`
    offset=`echo "$COMP_LINE" | grep -o -E " (0x)?[0-9a-fA-F]+ "`

    if [ -z $dev ]; then
    	devlist=$(_get_eb_dev);
    	COMPREPLY=( $(compgen -W "${devlist}" -- $cur) )
    elif [ -z $cpu ]; then
    	COMPREPLY=()
    elif [ -z $offset ]; then
    	COMPREPLY=()
   	elif [ -z $file ]; then
    	COMPREPLY=( $( compgen -f -X '!*.bin' -- $cur ) )
   	fi

    return 0

}

complete -F _eb_tools eb-ls
complete -F _eb_tools eb-info
complete -F _eb_tools eb-console
complete -F _eb_tools eb-read
complete -F _eb_tools eb-write
complete -F _eb_tools eb-put
complete -F _eb_tools eb-get
complete -F _eb_tools eb-mon
complete -F _eb_tools eb-reset
complete -F _eb_tools eb-find

complete -o filenames -F _eb_flash eb-flash
complete -o filenames -F _eb_fwload eb-fwload

