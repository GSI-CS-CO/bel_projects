# Rocky 9 Support

Q: Why do we need this?

A: To build our FPGA images and Etherbone. Rocky 9 does not provide libpng12 or lsb_release.

Q: How to use this?

A: You need to generate soft links and add paths to your environment variables.

<pre>
./generate_soft_links.sh
export PATH=$PATH:$(pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../../ip_cores/etherbone-core/api/.libs # make saftlib
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$(pwd)/../../ip_cores/etherbone-core/api/ # make saftlib
export CPATH=$CPATH:$(pwd)/../../ip_cores/etherbone-core/api/ # make saflib
</pre>

