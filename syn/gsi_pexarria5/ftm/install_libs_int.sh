cp /usr/lib64/libicudata*                      /common/export/timing-rte/generator-int/lib64/
cp /usr/lib64/libicui18n*                      /common/export/timing-rte/generator-int/lib64/
cp /usr/lib64/libicuuc*                        /common/export/timing-rte/generator-int/lib64/
cp ../../../../boost_1_69_0/installation/lib/libboost_*  /common/export/timing-rte/generator-int/lib64/
cp ../../../modules/ftm/lib/libcarpedm.so      /common/export/timing-rte/generator-int/lib/
cp dm-cmd                                      /common/export/timing-rte/generator-int/bin/
cp dm-sched                                    /common/export/timing-rte/generator-int/bin/
cp dm-dump.sh                                  /common/export/timing-rte/generator-int/


chmod g+w /common/export/timing-rte/generator-int/lib64/*
chmod g+w /common/export/timing-rte/generator-int/lib/*
chmod g+w /common/export/timing-rte/generator-int/bin/*
