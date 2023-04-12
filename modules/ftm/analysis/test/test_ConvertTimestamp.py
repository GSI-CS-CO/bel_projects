#! /usr/bin/env python3

import unittest
import CommandsHistory as C
from datetime import datetime

"""
Call the folder with CommandHistory.py:
python3 -m unittest testConvertTimestamp.py -v
"""
class TestConvertTimestamp(unittest.TestCase):
  def test_date(self):
    dt = C.convertTimestamp("2021-04-17")
    self.assertEqual(dt, datetime(2021,4,17))

  def test_dateHourMinute(self):
    dt = C.convertTimestamp("2021-02-04 12:00")
    self.assertEqual(dt, datetime(2021,2,4,12,0))

  def test_dateHourMinute1(self):
    dt = C.convertTimestamp("2021-02-06 0:0")
    self.assertEqual(dt, datetime(2021,2,6,0,0))

  def test_datetime(self):
    dt = C.convertTimestamp("2021-04-17 14:11:06")
    self.assertEqual(dt, datetime(2021,4,17,14,11,6))

  def test_datetimeWrong(self):
    dt = C.convertTimestamp("2021-14-17 14:11:06")
    self.assertEqual(dt, datetime(1,1,1))

  def test_datetimeseconds1(self):
    dt = C.convertTimestamp("2021-04-17 14:11:6.5")
    self.assertEqual(dt, datetime(2021,4,17,14,11,6,500000))

  def test_datetimeseconds3(self):
    dt = C.convertTimestamp("2021-04-17 14:11:6.345")
    self.assertEqual(dt, datetime(2021,4,17,14,11,6,345000))

  def test_datetimeseconds3a(self):
    dt = C.convertTimestamp("2021-04-17 14:11:6.000345")
    self.assertEqual(dt, datetime(2021,4,17,14,11,6,345))

  def test_datetimeseconds6(self):
    dt = C.convertTimestamp("2021-04-17 14:11:6.123456")
    self.assertEqual(dt, datetime(2021,4,17,14,11,6,123456))

  def test_dateWrong(self):
    dt = C.convertTimestamp("2021---+++")
    self.assertEqual(dt, datetime(1,1,1))
