#! /bin/bash
PYTHON_SITE=lib/python2.7/site-packages
mkdir -p $PYTHON_SITE
cp stuff/setuptools-16.0.zip ip_cores/hdl-make/setuptools-16.0.zip
cd ip_cores/hdl-make/ ; PYTHONPATH=../../$PYTHON_SITE python2.7 setup.py install --prefix=../..
rm setuptools-16.0.zip
