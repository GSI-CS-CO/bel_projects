Xena BASH Tools
===============

Synopsis
--------

Simple BASH script collection for XenaScripting (L2-3).

How to run
----------

Syntax:
<pre>
  ./xena_parser.sh {telnet connection} {test file}
</pre>

Example (Ext port, get SFP status):
<pre>
  ./xena_parser.sh ext test_cases/get_sfp_status.txt 
</pre>

Network Interfaces Setup
------------------------

File /etc/network/interfaces
<pre>
  # Use this for Xena Mgmt port
  auto eth3
  iface eth3 inet static
    address 192.168.1.171
    netmask 255.255.255.0
  
  # Use this for Xena Ext port
  auto eth4
  iface eth4 inet static
    address 172.16.255.201
    netmask 255.255.255.0
</pre>
