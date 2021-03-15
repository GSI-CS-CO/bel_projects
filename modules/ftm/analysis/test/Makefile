.PHONY: test fast

all: test convertTime
	
test:
	PYTHONPATH=../ python3 -m unittest testCommandsHistory.py

convertTime:
	PYTHONPATH=../ python3 -m unittest testConvertTimestamp.py

fast:
	PYTHONPATH=../ python3 -m unittest testCommandsHistory.py --failfast

