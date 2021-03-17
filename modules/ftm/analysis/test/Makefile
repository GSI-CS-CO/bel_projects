.PHONY: test fast

all: test convertTime
	
test:
	PYTHONPATH=../ python3 -m unittest testCommandsHistory.py $(ARGS)

convertTime:
	PYTHONPATH=../ python3 -m unittest testConvertTimestamp.py

fast:
	PYTHONPATH=../ python3 -m unittest testCommandsHistory.py --failfast

