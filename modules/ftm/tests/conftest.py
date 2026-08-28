import pytest

def pytest_addoption(parser):
    parser.addoption(
        "--runslow", action="store_true", default=False, help="run slow tests"
    )
    parser.addoption(
        "--development", action="store_true", default=False, help="run tests under developement"
    )

def pytest_configure(config):
    config.addinivalue_line("markers", "slow: mark test as slow to run")
    config.addinivalue_line("markers", "development: mark test as under development")

def pytest_collection_modifyitems(config, items):
    # --runslow NOT given in cli: skip slow tests
    if not config.getoption("--runslow"):
      skip_slow = pytest.mark.skip(reason="need --runslow option to run")
      for item in items:
          if "slow" in item.keywords:
              item.add_marker(skip_slow)
    # --development NOT given in cli: skip tests under development
    if not config.getoption("--development"):
      skip_slow = pytest.mark.skip(reason="need --development option to run")
      for item in items:
          if "development" in item.keywords:
              item.add_marker(skip_slow)
