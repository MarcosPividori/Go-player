#!/usr/bin/python

import subprocess
import os.path

output = open('stats.txt','a',0)
col_sizes = [3,4,10,6,5,8,7,7,7,7,8,5,5,7,7,6,6,20]
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

def marcos_go_command(
                    patterns=None,
                    bandit_coeff=None,
                    amaf_coeff=None,
                    fill_board=None,
                    long_game_coeff=None,
                    cycles_mcts=None,
                    threads_mcts=None,
                    limit_expansion=None,
                    root_parallel=False,
                    chinese_rules=False,
                    rave=True,
                    totally_random=False):
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
    if root_parallel:
        command+=" --root_parallel "
    if not chinese_rules:
        command+=" --japanese_rules "
    if totally_random:
        command+=" --totally_random_sim "
    if not rave:
        command+=" --no_rave "
    return command

def analyze_program(isWhite=False,
                    patterns=None,
                    bandit_coeff=None,
                    amaf_coeff=None,
                    fill_board=None,
                    long_game_coeff=None,
                    cycles_mcts=None,
                    threads_mcts=None,
                    limit_expansion=None,
                    root_parallel=False,
                    chinese_rules=False,
                    rave=True,
                    totally_random=False,
                    opponent_command=None,
                    komi=5,
                    size=9,
                    num_games=100):
    if isWhite:
      prog='WHITE'
      opp='BLACK'
    else:
      prog='BLACK'
      opp='WHITE'
    if opponent_command:
      os.environ[opp]=opponent_command
    else:
      if chinese_rules:
        os.environ[opp]="gnugo --mode gtp --capture-all-dead --chinese-rules"
      else:
        os.environ[opp]="gnugo --mode gtp --capture-all-dead"
    command= marcos_go_command(
                    patterns,
                    bandit_coeff,
                    amaf_coeff,
                    fill_board,
                    long_game_coeff,
                    cycles_mcts,
                    threads_mcts,
                    limit_expansion,
                    root_parallel,
                    chinese_rules,
                    rave,
                    totally_random)
    os.environ[prog]=command
    twogtp="gogui-twogtp -black \"$BLACK\" -white \"$WHITE\" -games "+str(num_games)+" -size "+str(size)+" -komi "+str(komi)+" -auto -sgffile ./game/stats"
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
                        str(size),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(cycles_mcts),
                        str(threads_mcts),
                        str(limit_expansion),
                        str(komi),
                        str(root_parallel),
                        str(chinese_rules),
                        str(totally_random),
                        str(rave),
                        str(patterns)])
    else:
      write_row(output,[str(counter),
                        prog[0],
                        'ERROR:',
                        'COD'+str(p.returncode),
                        str(size),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(cycles_mcts),
                        str(threads_mcts),
                        str(limit_expansion),
                        str(komi),
                        str(root_parallel),
                        str(chinese_rules),
                        str(totally_random),
                        str(rave),
                        str(patterns)])

write_row(output,['#','COL','WRATE','TOTAL','SIZE','BCOEFF','ACOEFF','FBOARD','LGCOEF','CYCLES','THREADS','LEXP','KOMI','ROOT_P','CHIN_R','TRAND','RAVE','PATT']);
#               isWhite  patterns         bandit_coeff  amaf_coeff  fill_board  long_game_coeff  cycles_mcts  threads_mcts)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0.2,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0.4,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0.6,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0.8,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  1,            1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  1.2,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  1.4,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  1.6,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  1.8,          1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  2,            1000,       6,          3,               30000,       5)

#analyze_program(True,    'patterns.txt',  0,              10,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,             100,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,             500,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            3000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            3500,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            5000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            7500,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,           10000,       6,          3,               30000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       8,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       4,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       2,          3,               30000,       5)

#analyze_program(True,    'patterns.txt',  0,            2500,       6,          3,               60000,       5)
#analyze_program(True,    'patterns.txt',  0,            3000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            5000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            4000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          3,               30000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               30000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               30000,       5)

#analyze_program(False,   'patterns.txt',  0,            2500,       6,          3,               70000,       5)

#RAVE vs NO-RAVE
analyze_program(isWhite=False, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=1000,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=True,
opponent_command=marcos_go_command(
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=2500,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=False,
  totally_random=True
)
)

analyze_program(isWhite=True, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=2500,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=True,
opponent_command=marcos_go_command(
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=1000,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=False,
  totally_random=True
)
)

#RAVE-KNOWLEDGE vs RAVE-RANDOM
analyze_program(isWhite=False, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=1000,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=False,
opponent_command=marcos_go_command(
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=2500,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=True
)
)

analyze_program(isWhite=True, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=2500,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=False,
opponent_command=marcos_go_command(
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=1000,
  fill_board=1,
  long_game_coeff=3,
  cycles_mcts=70000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=True
)
)

#analyze_program(True,    'patterns.txt',  0,             2500,       6,         3,               30000,       5, komi=5)
#analyze_program(True,    'patterns.txt',  0,             1000,       6,         3,                70000,       5)
#analyze_program(False,   'patterns.txt',  0,             1000,       6,         3,                30000,       5)
#analyze_program(False,   'patterns.txt',  0,             1000,       6,         3,                70000,       5)
#analyze_program(False,   'patterns.txt',  0,             2500,       6,         3,                70000,       5)

#analyze_program(False,   'patterns.txt',  0,            1000,       6,          3,               70000,       5)
#analyze_program(False,   'patterns.txt',  0,            5000,       6,          3,               70000,       5)
#analyze_program(True,    'patterns.txt',  0,            2500,       6,          3,               70000,       5)

#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               65000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               55000,       5)
#analyze_program(True,    'patterns.txt',  0,            1000,       6,          3,               45000,       5)

output.close()
 
