#! /bin/bash
PYTHON_SITE=lib/python2.7/site-packages
mkdir -p $PYTHON_SITE
cd ip_cores/hdl-make/ ; PYTHONPATH=../../$PYTHON_SITE python2.7 setup.py install --prefix=../..
