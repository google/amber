#!/usr/bin/env python

# Copyright 2020 The Amber Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#	http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Script to check files for inclusive language. The script will scan all files
and flag non-inclusive terminology which is identified.

Usage, run the script from a folder and the script will scan down through that
folder.
"""

import fnmatch
import os
import re
import sys

REGEXES = [
	r"(?i)black[-_]?list",
	r"(?i)white[-_]?list",
	r"(?i)gr[ea]y[-_]?list",
	r"(?i)(first class citizen)",
	r"(?i)black[-_]?hat",
	r"(?i)white[-_]?hat",
	r"(?i)gr[ea]y[-_]?hat",
	r"(?i)master",
	r"(?i)slave",
	r"(?i)\bhim\b",
	r"(?i)\bhis\b",
	r"(?i)\bshe\b",
	r"(?i)\bher\b",
	r"(?i)\bhers\b",
	r"(?i)\bman\b",
	r"(?i)\bwoman\b",
	r"(?i)\she\s",
	r"(?i)\she$",
	r"(?i)^he\s",
	r"(?i)^he$",
	r"(?i)\she['|\u2019]d\s",
	r"(?i)\she['|\u2019]d$",
	r"(?i)^he['|\u2019]d\s",
	r"(?i)^he['|\u2019]d$",
	r"(?i)\she['|\u2019]s\s",
	r"(?i)\she['|\u2019]s$",
	r"(?i)^he['|\u2019]s\s",
	r"(?i)^he['|\u2019]s$",
	r"(?i)\she['|\u2019]ll\s",
	r"(?i)\she['|\u2019]ll$",
	r"(?i)^he['|\u2019]ll\s",
	r"(?i)^he['|\u2019]ll$",
	r"(?i)grandfather",
	r"(?i)\bmitm\b",
	r"(?i)\bcrazy\b",
	r"(?i)\binsane\b",
	r"(?i)\bblind\sto\b",
	r"(?i)\bflying\sblind\b",
	r"(?i)\bblind\seye\b",
	r"(?i)\bcripple\b",
	r"(?i)\bcrippled\b",
	r"(?i)\bdumb\b",
	r"(?i)\bdummy\b",
	r"(?i)\bparanoid\b",
	r"(?i)\bsane\b",
	r"(?i)\bsanity\b",
	r"(?i)red[-_]?line",
]

SUPPRESSIONS = [
	r"(?i)MS_SLAVE",
	r"(?i)man[ -_]?page",
]


REGEX_LIST = []
for reg in REGEXES:
	REGEX_LIST.append(re.compile(reg))

SUPPRESSION_LIST = []
for supp in SUPPRESSIONS:
	SUPPRESSION_LIST.append(re.compile(supp))

def find(top, filename_glob, skip_glob_list):
	"""Returns files in the tree rooted at top matching filename_glob but not
	in directories matching skip_glob_list."""

	file_list = []
	for path, dirs, files in os.walk(top):
		for glob in skip_glob_list:
			for match in fnmatch.filter(dirs, glob):
				dirs.remove(match)
		for filename in fnmatch.filter(files, filename_glob):
			if filename == os.path.basename(__file__):
				continue
			file_list.append(os.path.join(path, filename))
	return file_list


def filtered_descendants(glob):
	"""Returns glob-matching filenames under the current directory, but skips
	some irrelevant paths."""
	return find('.', glob, ['third_party', 'external', 'build*', 'out*',
							'CompilerIdCXX', '.git'])

def check_match(filename, contents):
	"""Check if contents contains any matching entries"""
	ret = False
	for reg in REGEX_LIST:
		match = reg.search(contents)
		if match:
			suppressed = False
			for supp in SUPPRESSION_LIST:
				idx = match.start()
				supp_match = supp.match(contents[idx:])
				if supp_match:
					suppressed = True

				# This is a hack to handle the MS_ prefix that is needed
				# to check for. Find a better way if we get more suppressions
				# which modify the prefix of the string
				if idx >= 3:
					supp_match = supp.match(contents[idx - 3:])
					if supp_match:
						suppressed = True

			if not suppressed:
				# No matching suppression.
				print("{}: found non-inclusive language: {}".format(
						filename, match.group(0)))
				ret = True

	return ret


def alert_if_lang_matches(glob):
	"""Prints names of all files matching non-inclusive language.

	Finds all glob-matching files under the current directory and checks if they
	contain the language pattern.  Prints the names of all the files that
	match.

	Returns the total number of file names printed.
	"""
	verbose = False
	printed_count = 0
	for file in filtered_descendants(glob):
		has_match = False
		try:
			with open(file, 'r', encoding='utf8') as contents:
				if check_match(file, contents.read()):
					printed_count += 1
		except:
			if verbose:
				print("skipping {}".format(file))

	return printed_count


def main():
	globs = ['*']
	count = 0
	for glob in globs:
		count += alert_if_lang_matches(glob)

	sys.exit(count > 0)

if __name__ == '__main__':
	main()
