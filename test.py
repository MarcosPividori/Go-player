#!/usr/bin/python

from stats import *

begin_stats()

analyze_program(isWhite=False, komi=5, num_games=1, patterns='patterns.txt')

end_stats()
