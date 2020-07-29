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

from __future__ import print_function

import fnmatch
import os
import re
import sys

REGEXES = [
	r"(?i)(black|white|gr[ea]y)[-_]?list",
	r"(?i)(first class citizen)",
	r"(?i)(black|white|gr[ea]y)[-_]?hat",
	r"(?i)(master|slave)",
	r"(?i)\b(him|his|she|her|hers|man|woman)\b",
	r"(?i)\s(he|he'd|he's|he'll)\s",
	r"(?i)grandfather",
	r"(?i)\bmitm\\b",
	r"(?i)native",
	r"(?i)\b(crazy|insane|blind\sto|flying\sblind|blind\seye|cripple|crippled|dumb|dummy|paranoid)\b",
	r"(?i)\b(sane|sanity)\b",
	r"(?i)red[-_]?line",
]

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


def alert_if_lang_matches(glob):
    """Prints names of all files matching non-inclusive language.

    Finds all glob-matching files under the current directory and checks if they
    contain the language pattern.  Prints the names of all the files that
    match.

    Returns the total number of file names printed.
    """

    reg_list = []
    for reg in REGEXES:
    	reg_list.append(re.compile(reg))

    printed_count = 0
    for file in filtered_descendants(glob):
        has_match = False
        with open(file) as contents:
            for line in contents:
            	for reg in reg_list:
            		match = reg.search(line)
            		if match:
            			print(file, ': found non-inclusive language:', match.group(0))
            			printed_count += 1

    return printed_count


def main():
    globs = ['*']
    count = 0
    for glob in globs:
        count += alert_if_lang_matches(glob)

    sys.exit(count > 0)

if __name__ == '__main__':
  main()
