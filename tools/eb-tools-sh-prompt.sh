#!/bin/bash

_get_eb_dev()
{
	local dm_devlist
	dm_devlist=`history | cut -c 8- | grep -v "grep" | grep "\(eb\-\)[a-zA-Z0-9\-\_]*" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`

	echo $dm_devlist
}
 _eb_tools()
{
    local cur compline devlist dev
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    compline="${COMP_WORDS[*]}"
		dev=`echo "$compline" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`

    if [ -z $dev ]; then
    	devlist=$(_get_eb_dev);
    	COMPREPLY=( $(compgen -W "${devlist}" -- $cur) )
   	fi

    return 0

}

 _eb_flash()
{
    local cur devlist dev compline file
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    compline="${COMP_WORDS[*]}"

		dev=`echo "$compline" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`
		file=`echo "$compline" | grep -o -E "!*.rpd"`

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

    local cur devlist dev compline file cpu offset
  	COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    compline="${COMP_WORDS[*]}"
		dev=`echo "$compline" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`
		file=`echo "$compline" | grep -o -E "!*.bin"`
    cpu=`echo "$compline" | grep -o -E " (u[0-9]+|w|) "`
    offset=`echo "$compline" | grep -o -E " (0x)?[0-9a-fA-F]+ "`

    #echo "!$cpu!"
    #echo "?$offset?"
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

