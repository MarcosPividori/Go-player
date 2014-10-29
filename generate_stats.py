#!/usr/bin/python

import subprocess
import os.path

output = open('stats.txt','a')
col_sizes = [3,4,10,6,8,7,7,7,7,8,20]
counter=0

def write_row(files,l):
    for i in range(0,len(l)):
      el=l[i]
      if len(el)< col_sizes[i] :
        el = el + " " * (col_sizes[i]-len(el))
      files.write(el)
    files.write('\n')

def winning_rate(prog):
    stats = open('./game/stats.dat','r')
    wcount=0;
    bcount=0;
    for l in stats:
      if l[0]=='#':
        continue
      if l.count('W')>=2:
        wcount+=1
      elif l.count('B')>=2:
        bcount+=1
    stats.close()
    if wcount+bcount == 0:
        return (0,0)
    if prog == 'WHITE':
      return (wcount/float(wcount+bcount),wcount+bcount)
    else:
      return (bcount/float(wcount+bcount),wcount+bcount)

def analyze_program(isWhite=False,
                    patterns=None,
                    bandit_coeff=None,
                    amaf_coeff=None,
                    fill_board=None,
                    long_game_coeff=None,
                    cycles_mcts=None,
                    threads_mcts=None,
                    limit_expansion=None):
    if isWhite:
      prog='WHITE'
      gnu='BLACK'
    else:
      prog='BLACK'
      gnu='WHITE'
    os.environ[gnu]="gnugo --mode gtp --capture-all-dead"
    command="./bin/marcos_go "
    if patterns!=None:
        command+=" --patterns " + patterns
    if bandit_coeff!=None:
        command+=" --bandit_coeff " + str(bandit_coeff)
    if amaf_coeff!=None:
        command+=" --amaf_coeff " + str(amaf_coeff)
    if fill_board!=None:
        command+=" --fill_board " + str(fill_board)
    if long_game_coeff!=None:
        command+=" --long_game_coeff " + str(long_game_coeff)
    if cycles_mcts!=None:
        command+=" --cycles_mcts " + str(cycles_mcts)
    if threads_mcts!=None:
        command+=" --threads_mcts " + str(threads_mcts)
    if limit_expansion!=None:
        command+=" --limit_expansion " + str(limit_expansion)
    os.environ[prog]=command
    twogtp="gogui-twogtp -black \"$BLACK\" -white \"$WHITE\" -games 100 -size 9 -komi 5 -auto -sgffile ./game/stats"
    if os.path.isfile('./game/stats.dat'):
       subprocess.Popen("rm ./game/stats*",shell=True)
    p=subprocess.Popen(twogtp,shell=True)
    p.wait()
    if p.returncode == 0 :
      (wrate,total) = winning_rate(prog)
      write_row(output,[str(counter),
                        prog[0],
                        "{0:.4f}".format(wrate),
                        str(total),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(cycles_mcts),
                        str(threads_mcts),
                        str(patterns)])
    else:
      write_row(output,[str(counter),
                        prog[0],
                        'ERROR:',
                        'COD'+str(p.returncode),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(cycles_mcts),
                        str(threads_mcts),
                        str(patterns)])

write_row(output,['#','COL','WRATE','TOTAL','BCOEFF','ACOEFF','FBOARD','LGCOEF','CYCLES','THREADS','PATT']);
#               isWhite  patterns         bandit_coeff  amaf_coeff  fill_board  long_game_coeff  cycles_mcts  threads_mcts)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0.2,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0.4,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0.6,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0.8,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  1,            1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  1.2,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  1.4,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  1.6,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  1.8,          1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  2,            1000,       6,          1.4,             30000,       5)

#analyze_program(True,    'patterns.txt',  0,              10,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,             100,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,             500,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            3000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            3500,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            5000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            7500,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,           10000,       6,          1.4,             30000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       8,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       4,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       2,          1.4,             30000,       5)

#analyze_program(True,    'patterns.txt',  0,            2500,       6,          1.4,             60000,       5)
#analyze_program(True,    'patterns.txt',  0,            3000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            5000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            4000,       6,          1.4,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          1.4,             30000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       6,            1,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.8,             30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          2.2,             30000,       5)

#analyze_program(False,   'patterns.txt',  0,            2500,       6,          1.4,             70000,       5)

analyze_program(False,   'patterns.txt',  0,             1000,       6,          1.4,             30000,       5)
analyze_program(False,   'patterns.txt',  0,             1000,       6,          1.4,             30000,       5)
#analyze_program(False,   'patterns.txt',  0,             2500,       6,          1.4,             70000,       5)

#analyze_program(False,   'patterns.txt',  0,            1000,       6,          1.4,             70000,       5)
#analyze_program(False,   'patterns.txt',  0,            5000,       6,          1.4,             70000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          1.4,             70000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.4,             65000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.4,             55000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          1.4,             45000,       5)

output.close()
 
