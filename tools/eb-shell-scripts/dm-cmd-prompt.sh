#!/bin/bash

# Autocomplete for dm-cmd
# !!! requires carpeDM >= v0.26.0!!!
# This calls dm-sched once in the background to obtain pattern and node names from DM
#
#
# CAVEAT: when using the SSH wrapper script eb-ssh-func.sh, overriding SSH for dm-cmd only works with a preceding
# backslash, i.e. '\dm-cmd'.  Calls like 'command dm-cmd', etc. will not work. See eb-ssh-func.sh for details
#
# Install:
# this file belongs in /etc/bash_completion.d/
#
# After copying the file, you need to either restart the shell or source the file:
# . /etc/bash_completion.d/dm-cmd-prompt.sh
#



#Calling dm-sched only once requires globals to store the data persistently
__dm_schedule_dump=''
__dm_patterns=''
__dm_blocks=''
__dm_nodes=''
__dm_cmd_choice=''


 _dm-cmd()
{


    local cur prev devlist dev cmdlist cmd cmdpattern schedargcnt nossh
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    dev=`echo "$COMP_LINE" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]* "`
    nossh=`echo "$COMP_LINE" | grep -o "[\\]dm-cmd"` #if user wants to override the ssh wrap of eb-ssh-func.sh, dm-cmd was called with preceding backslash

    cmdlist="halt start abort stop flow flush noop startpattern abortpattern stoppattern flowpattern staticflushpattern queue hex origin diag cfghwdiag cfgcpudiag cleardiag clearhwdiag clearcpudiag starthwdiag stophwdiag"
    cmdpattern="(halt|start|abort|stop|flow|flush|noop|startpattern|abortpattern|stoppattern|flowpattern|staticflushpattern|queue|hex|origin|diag|cfghwdiag|cfgcpudiag|cleardiag|clearhwdiag|clearcpudiag|starthwdiag|stophwdiag) "
    cmd=`echo "$COMP_LINE" | grep -o -E "$cmdpattern" | cut -d\  -f1`
    schedargcnt=0
    #check if nodes or patterns are already in word queue. increase schedargcnt for each queue entry
    for ((i=0; i<$COMP_CWORD; i++));
    do
      if [[ "$__dm_patterns" =~ "${COMP_WORDS[i]}" ]] || [[ "$__dm_nodes" =~ "${COMP_WORDS[i]}" ]]; then
        let "schedargcnt = $schedargcnt +1"
      fi
    done

    if [ -z $dev ]; then #1. no eb-device, find one
      devlist=`history | cut -c 8- | grep -v "grep" | grep "\(dm\-\)[a-zA-Z0-9\-\_]*" | grep -o -E "(dev|tcp|udp)\/.[a-zA-Z0-9\_\-\.]*"`
      COMPREPLY=( $(compgen -W "${devlist}" -- $cur) )
      __dm_schedule_dump= # clear schedule dump for next call
      __dm_patterns=
      __dm_blocks=
      __dm_nodes=
    elif [ -z $cmd ]; then # 2. eb-device, but no cmd. find one
        COMPREPLY=( $(compgen -W "${cmdlist}" -- $cur) )
        __dm_cmd_choice="${COMPREPLY}"
        # unsing $__dm_cmd_choice may seem redundant, but is safer than $cmd in case another word resembling a command is inserted later
    elif [ -n "$dev" ]; then # 3. eb-dev, but no dump. dump from dev
      if [ -z "$__dm_schedule_dump" ]; then
        if [ -n "$nossh" ]; then
          __dm_schedule_dump=`command dm-sched ${dev} dump -s | grep -E -v "(\->|graph|node)"`;
        else
          __dm_schedule_dump=`dm-sched ${dev} dump -s | grep -E -v "(\->|graph|node)"`;
        fi

        __dm_patterns=`echo "$__dm_schedule_dump" | grep -o "pattern\=\"[a-zA-Z0-9\_]*" | cut -d\" -f2`;
        __dm_blocks=`echo "$__dm_schedule_dump" | grep "type=\"block\"" | grep -o "^[a-zA-Z0-9\_]*"`;
        __dm_nodes=`echo "$__dm_schedule_dump" | grep -o "^[a-zA-Z0-9\_]*"`
      fi

      if [ -n "$cmd" ]; then # 4. cmd and dump
        #no match of word queue and schedule yet, choose first argument pattern/block/node according to cmd
        if [ "$schedargcnt" -eq 0 ]; then
          case "$__dm_cmd_choice" in
            startpattern|stoppattern|abortpattern|flowpattern|flushpattern|staticflushpattern)
              COMPREPLY=( $(compgen -W "${__dm_patterns}" -- $cur) )
            ;;
            flow|stop|flush|staticflush|noop|queue)

              COMPREPLY=( $(compgen -W "${__dm_blocks}" -- $cur) )
            ;;
            hex)
              COMPREPLY=( $(compgen -W "${__dm_nodes}" -- $cur) )
            ;;
            *)
              COMPREPLY=()
            ;;
          esac
        fi

        if [ "$schedargcnt" -eq 1 ]; then
          #already one match of word queue and schedule, choose second argument pattern/block/node according to cmd
          case "$__dm_cmd_choice" in
            flowpattern)
              COMPREPLY=( $(compgen -W "${__dm_patterns}" -- $cur) )
            ;;
            flow)
              COMPREPLY=( $(compgen -W "${__dm_nodes}" -- $cur) )
            ;;
            *)
            COMPREPLY=()
            ;;
          esac
        fi

      fi
        #2+ matches of COMP_LINE and schedule, do nothing
    fi

    return 0

}
complete -F _dm-cmd dm-cmd
complete -F _dm-cmd \\dm-cmd # make \dm-cmd dedicated command, so we can then check the call for the backslash
