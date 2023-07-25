import pytest

def pytest_addoption(parser):
    parser.addoption(
        "--runslow", action="store_true", default=False, help="run slow tests"
    )
    parser.addoption(
        "--runthread32", action="store_true", default=False, help="run tests for 32 threads"
    )

def pytest_configure(config):
    config.addinivalue_line("markers", "slow: mark test as slow to run")
    config.addinivalue_line("markers", "thread32: mark as test for 32 threads")

def pytest_collection_modifyitems(config, items):
    # --runslow NOT given in cli: skip slow tests
    if not config.getoption("--runslow"):
      skip_slow = pytest.mark.skip(reason="need --runslow option to run")
      for item in items:
          if "slow" in item.keywords:
              item.add_marker(skip_slow)
    # --runthread NOT given in cli: skip tests for 32 threads
    if not config.getoption("--runthread32"):
      skip_thread32 = pytest.mark.skip(reason="need --runthread32 option to run")
      for item in items:
          if "thread32" in item.keywords:
              item.add_marker(skip_thread32)
