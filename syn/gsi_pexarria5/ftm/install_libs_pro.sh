cp /usr/lib64/libicudata*                      /common/export/timing-rte/generator-pro/lib64/
cp /usr/lib64/libicui18n*                      /common/export/timing-rte/generator-pro/lib64/
cp /usr/lib64/libicuuc*                        /common/export/timing-rte/generator-pro/lib64/
cp /usr/lib64/libboost_*                       /common/export/timing-rte/generator-pro/lib64/
cp ../../../modules/ftm/lib/libcarpedm.so      /common/export/timing-rte/generator-pro/lib/
cp dm-cmd                                      /common/export/timing-rte/generator-pro/bin/
cp dm-sched                                    /common/export/timing-rte/generator-pro/bin/

chmod g+w /common/export/timing-rte/generator-pro/lib64/*
chmod g+w /common/export/timing-rte/generator-pro/lib/*
chmod g+w /common/export/timing-rte/generator-pro/bin/*
