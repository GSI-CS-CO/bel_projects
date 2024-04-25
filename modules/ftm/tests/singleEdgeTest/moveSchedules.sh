#! /usr/bin/bash

mkdir -p dot/
LD_LIBRARY_PATH=../lib ./singleEdgeTest/singleEdgeTest -s

mv dot/testSingleEdge-block-block-altdst.dot schedules/
mv dot/testSingleEdge-block-block-defdst.dot schedules/
mv dot/testSingleEdge-block-blockalign-altdst.dot schedules/
mv dot/testSingleEdge-block-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-block-flow-altdst.dot schedules/
mv dot/testSingleEdge-block-flow-defdst.dot schedules/
mv dot/testSingleEdge-block-flush-altdst.dot schedules/
mv dot/testSingleEdge-block-flush-defdst.dot schedules/
mv dot/testSingleEdge-block-noop-altdst.dot schedules/
mv dot/testSingleEdge-block-noop-defdst.dot schedules/
mv dot/testSingleEdge-block-origin-altdst.dot schedules/
mv dot/testSingleEdge-block-origin-defdst.dot schedules/
mv dot/testSingleEdge-block-startthread-altdst.dot schedules/
mv dot/testSingleEdge-block-startthread-defdst.dot schedules/
mv dot/testSingleEdge-block-switch-altdst.dot schedules/
mv dot/testSingleEdge-block-switch-defdst.dot schedules/
mv dot/testSingleEdge-block-tmsg-altdst.dot schedules/
mv dot/testSingleEdge-block-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-block-wait-altdst.dot schedules/
mv dot/testSingleEdge-block-wait-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-block-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-block-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-blockalign-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-flow-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-flow-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-flush-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-flush-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-noop-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-noop-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-origin-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-origin-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-startthread-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-startthread-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-switch-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-switch-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-tmsg-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-blockalign-wait-altdst.dot schedules/
mv dot/testSingleEdge-blockalign-wait-defdst.dot schedules/
mv dot/testSingleEdge-flow-block-defdst.dot schedules/
mv dot/testSingleEdge-flow-block-flowdst.dot schedules/
mv dot/testSingleEdge-flow-block-target.dot schedules/
mv dot/testSingleEdge-flow-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-flow-blockalign-flowdst.dot schedules/
mv dot/testSingleEdge-flow-blockalign-target.dot schedules/
mv dot/testSingleEdge-flow-flow-defdst.dot schedules/
mv dot/testSingleEdge-flow-flow-flowdst.dot schedules/
mv dot/testSingleEdge-flow-flush-defdst.dot schedules/
mv dot/testSingleEdge-flow-flush-flowdst.dot schedules/
mv dot/testSingleEdge-flow-noop-defdst.dot schedules/
mv dot/testSingleEdge-flow-noop-flowdst.dot schedules/
mv dot/testSingleEdge-flow-origin-defdst.dot schedules/
mv dot/testSingleEdge-flow-origin-flowdst.dot schedules/
mv dot/testSingleEdge-flow-startthread-defdst.dot schedules/
mv dot/testSingleEdge-flow-startthread-flowdst.dot schedules/
mv dot/testSingleEdge-flow-switch-defdst.dot schedules/
mv dot/testSingleEdge-flow-switch-flowdst.dot schedules/
mv dot/testSingleEdge-flow-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-flow-tmsg-flowdst.dot schedules/
mv dot/testSingleEdge-flow-wait-defdst.dot schedules/
mv dot/testSingleEdge-flow-wait-flowdst.dot schedules/
mv dot/testSingleEdge-flush-block-defdst.dot schedules/
mv dot/testSingleEdge-flush-block-flushovr.dot schedules/
mv dot/testSingleEdge-flush-block-target.dot schedules/
mv dot/testSingleEdge-flush-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-flush-blockalign-flushovr.dot schedules/
mv dot/testSingleEdge-flush-blockalign-target.dot schedules/
mv dot/testSingleEdge-flush-flow-defdst.dot schedules/
mv dot/testSingleEdge-flush-flow-flushovr.dot schedules/
mv dot/testSingleEdge-flush-flush-defdst.dot schedules/
mv dot/testSingleEdge-flush-flush-flushovr.dot schedules/
mv dot/testSingleEdge-flush-noop-defdst.dot schedules/
mv dot/testSingleEdge-flush-noop-flushovr.dot schedules/
mv dot/testSingleEdge-flush-origin-defdst.dot schedules/
mv dot/testSingleEdge-flush-origin-flushovr.dot schedules/
mv dot/testSingleEdge-flush-startthread-defdst.dot schedules/
mv dot/testSingleEdge-flush-startthread-flushovr.dot schedules/
mv dot/testSingleEdge-flush-switch-defdst.dot schedules/
mv dot/testSingleEdge-flush-switch-flushovr.dot schedules/
mv dot/testSingleEdge-flush-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-flush-tmsg-flushovr.dot schedules/
mv dot/testSingleEdge-flush-wait-defdst.dot schedules/
mv dot/testSingleEdge-flush-wait-flushovr.dot schedules/
mv dot/testSingleEdge-noop-block-defdst.dot schedules/
mv dot/testSingleEdge-noop-block-target.dot schedules/
mv dot/testSingleEdge-noop-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-noop-blockalign-target.dot schedules/
mv dot/testSingleEdge-noop-flow-defdst.dot schedules/
mv dot/testSingleEdge-noop-flush-defdst.dot schedules/
mv dot/testSingleEdge-noop-noop-defdst.dot schedules/
mv dot/testSingleEdge-noop-origin-defdst.dot schedules/
mv dot/testSingleEdge-noop-startthread-defdst.dot schedules/
mv dot/testSingleEdge-noop-switch-defdst.dot schedules/
mv dot/testSingleEdge-noop-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-noop-wait-defdst.dot schedules/
mv dot/testSingleEdge-origin-block-defdst.dot schedules/
mv dot/testSingleEdge-origin-block-origindst.dot schedules/
mv dot/testSingleEdge-origin-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-origin-blockalign-origindst.dot schedules/
mv dot/testSingleEdge-origin-flow-defdst.dot schedules/
mv dot/testSingleEdge-origin-flow-origindst.dot schedules/
mv dot/testSingleEdge-origin-flush-defdst.dot schedules/
mv dot/testSingleEdge-origin-flush-origindst.dot schedules/
mv dot/testSingleEdge-origin-noop-defdst.dot schedules/
mv dot/testSingleEdge-origin-noop-origindst.dot schedules/
mv dot/testSingleEdge-origin-origin-defdst.dot schedules/
mv dot/testSingleEdge-origin-origin-origindst.dot schedules/
mv dot/testSingleEdge-origin-startthread-defdst.dot schedules/
mv dot/testSingleEdge-origin-startthread-origindst.dot schedules/
mv dot/testSingleEdge-origin-switch-defdst.dot schedules/
mv dot/testSingleEdge-origin-switch-origindst.dot schedules/
mv dot/testSingleEdge-origin-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-origin-tmsg-origindst.dot schedules/
mv dot/testSingleEdge-origin-wait-defdst.dot schedules/
mv dot/testSingleEdge-origin-wait-origindst.dot schedules/
mv dot/testSingleEdge-startthread-block-defdst.dot schedules/
mv dot/testSingleEdge-startthread-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-startthread-flow-defdst.dot schedules/
mv dot/testSingleEdge-startthread-flush-defdst.dot schedules/
mv dot/testSingleEdge-startthread-noop-defdst.dot schedules/
mv dot/testSingleEdge-startthread-origin-defdst.dot schedules/
mv dot/testSingleEdge-startthread-startthread-defdst.dot schedules/
mv dot/testSingleEdge-startthread-switch-defdst.dot schedules/
mv dot/testSingleEdge-startthread-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-startthread-wait-defdst.dot schedules/
mv dot/testSingleEdge-switch-block-defdst.dot schedules/
mv dot/testSingleEdge-switch-block-switchdst.dot schedules/
mv dot/testSingleEdge-switch-block-target.dot schedules/
mv dot/testSingleEdge-switch-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-switch-blockalign-switchdst.dot schedules/
mv dot/testSingleEdge-switch-blockalign-target.dot schedules/
mv dot/testSingleEdge-switch-flow-defdst.dot schedules/
mv dot/testSingleEdge-switch-flow-switchdst.dot schedules/
mv dot/testSingleEdge-switch-flush-defdst.dot schedules/
mv dot/testSingleEdge-switch-flush-switchdst.dot schedules/
mv dot/testSingleEdge-switch-noop-defdst.dot schedules/
mv dot/testSingleEdge-switch-noop-switchdst.dot schedules/
mv dot/testSingleEdge-switch-origin-defdst.dot schedules/
mv dot/testSingleEdge-switch-origin-switchdst.dot schedules/
mv dot/testSingleEdge-switch-startthread-defdst.dot schedules/
mv dot/testSingleEdge-switch-startthread-switchdst.dot schedules/
mv dot/testSingleEdge-switch-switch-defdst.dot schedules/
mv dot/testSingleEdge-switch-switch-switchdst.dot schedules/
mv dot/testSingleEdge-switch-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-switch-tmsg-switchdst.dot schedules/
mv dot/testSingleEdge-switch-wait-defdst.dot schedules/
mv dot/testSingleEdge-switch-wait-switchdst.dot schedules/
mv dot/testSingleEdge-tmsg-block-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-block-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-block-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-block-reference.dot schedules/
mv dot/testSingleEdge-tmsg-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-blockalign-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-blockalign-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-blockalign-reference.dot schedules/
mv dot/testSingleEdge-tmsg-flow-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-flow-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-flow-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-flow-reference.dot schedules/
mv dot/testSingleEdge-tmsg-flush-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-flush-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-flush-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-flush-reference.dot schedules/
mv dot/testSingleEdge-tmsg-noop-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-noop-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-noop-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-noop-reference.dot schedules/
mv dot/testSingleEdge-tmsg-origin-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-origin-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-origin-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-origin-reference.dot schedules/
mv dot/testSingleEdge-tmsg-startthread-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-startthread-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-startthread-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-startthread-reference.dot schedules/
mv dot/testSingleEdge-tmsg-switch-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-switch-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-switch-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-switch-reference.dot schedules/
mv dot/testSingleEdge-tmsg-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-tmsg-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-tmsg-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-tmsg-reference.dot schedules/
mv dot/testSingleEdge-tmsg-wait-defdst.dot schedules/
mv dot/testSingleEdge-tmsg-wait-dynpar0.dot schedules/
mv dot/testSingleEdge-tmsg-wait-dynpar1.dot schedules/
mv dot/testSingleEdge-tmsg-wait-reference.dot schedules/
mv dot/testSingleEdge-wait-block-defdst.dot schedules/
mv dot/testSingleEdge-wait-block-target.dot schedules/
mv dot/testSingleEdge-wait-blockalign-defdst.dot schedules/
mv dot/testSingleEdge-wait-blockalign-target.dot schedules/
mv dot/testSingleEdge-wait-flow-defdst.dot schedules/
mv dot/testSingleEdge-wait-flush-defdst.dot schedules/
mv dot/testSingleEdge-wait-noop-defdst.dot schedules/
mv dot/testSingleEdge-wait-origin-defdst.dot schedules/
mv dot/testSingleEdge-wait-startthread-defdst.dot schedules/
mv dot/testSingleEdge-wait-switch-defdst.dot schedules/
mv dot/testSingleEdge-wait-tmsg-defdst.dot schedules/
mv dot/testSingleEdge-wait-wait-defdst.dot schedules/
