#!/bin/bash
xdot $1 -f neato --filterargs="-Gmodel=subset -Goverlap=compress"
