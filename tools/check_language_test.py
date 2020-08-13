#!/usr/bin/env python

# Copyright 2020 The Amber Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Unit tests for check_language.py."""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import check_language

class TestCheckLanguage(unittest.TestCase):
	def testMatches(self):
		tests = ["blacklist", "black-list", "black_list", "whitelist",
			"white-list", "white_list", "greylist", "grey-list", "grey_list",
			"graylist", "gray-list", "gray_list", "first class citizen",
			"blackhat", "black-hat", "black_hat", "whitehat", "white-hat",
			"white_hat", "greyhat", "grey-hat", "grey_hat", "grayhat",
			"gray-hat", "gray_hat", "master", "slave", "him", "his", "she",
			"her", "hers", "man", "woman", "he", "he'd", "he's", "he'll",
			"he\u2019d", "he\u2019s", "he\u2019ll",
			"grandfather", "mitm", "crazy", "insane", "blind to",
			"flying blind", "blind eye", "cripple", "crippled", "dumb",
			"dummy", "paranoid", "sane", "sanity", "redline", "red-line",
			"red_line"]

		for word in tests:
			self.assertTrue(
				check_language.check_match("", "this is a " + word + " attempt"), word)


	def testSuppression(self):
		self.assertFalse(check_language.check_match("", "in the man-pages"))
		self.assertFalse(check_language.check_match("", "the MS_SLAVE test"))


	def testMatchStartofFileWhenRequireSpace(self):
		self.assertTrue(check_language.check_match("", "he said"))


	def testMatchOverNewline(self):
		self.assertTrue(check_language.check_match("", "flying\nblind"))


if __name__ == '__main__':
	unittest.main()
