.PHONY: test fast clean

all: test convertTime
	
test:
	PYTHONPATH=../ python3 -m pytest testCommandsHistory.py $(ARGS)

convertTime:
	PYTHONPATH=../ python3 -m pytest testConvertTimestamp.py

fast:
	PYTHONPATH=../ python3 -m unittest testCommandsHistory.py --failfast

clean:
	rm -r CommandsHistory_Fri_Feb__5_13.33.22_2021*
