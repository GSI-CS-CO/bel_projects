.PHONY: test fast clean

all:
	PYTHONPATH=../ python3 -m pytest $(OPTIONS)

test:
	PYTHONPATH=../ python3 -m pytest test_CommandsHistory.py $(ARGS)

convertTime:
	PYTHONPATH=../ python3 -m pytest test_ConvertTimestamp.py

fast:
	PYTHONPATH=../ python3 -m unittest test_CommandsHistory.py --failfast

clean:
	rm -r CommandsHistory_Fri_Feb__5_13.33.22_2021*
