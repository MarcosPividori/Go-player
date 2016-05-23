#!/usr/bin/python

import subprocess
import os.path
from collections import OrderedDict

col_width = OrderedDict([
    ('COL'    , 4),
    ('WRATE'  , 10),
    ('TOTAL'  , 6),
    ('SIZE'   , 5),
    ('BCOEFF' , 8),
    ('ACOEFF' , 7),
    ('FBOARD' , 7),
    ('LGCOEF' , 7),
    ('PCF'    , 4),
    ('CCF'    , 4),
    ('ADC'    , 4),
    ('AEC'    , 4),
    ('LAT'    , 4),
    ('CYCLES' , 7),
    ('THREADS', 8),
    ('LEXP'   , 5),
    ('KOMI'   , 5),
    ('ROOT_P' , 7),
    ('CHIN_R' , 7),
    ('TRAND'  , 6),
    ('RAVE'   , 6),
    ('PATT'   , 15)
])

if not os.path.exists('./stats'):
    os.makedirs('./stats')

def write_row(file_path,data):
    with open(file_path,'a',0) as output:
      for key in col_width:
        el=str(data.get(key,""))
        el=el + " " * (col_width[key]-len(el))
        output.write(el)
      output.write('\n')

def write_stats(file_path,data):
    if not os.path.isfile(file_path):
        write_row(file_path,{x:x for x in col_width})
    write_row(file_path,data)

def winning_rate(prog):
    stats = open('./stats/stats.dat','r')
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

def analyze_program(
        isWhite=False,
        patterns=None,
        bandit_coeff=None,
        amaf_coeff=None,
        fill_board=None,
        long_game_coeff=None,
        pattern_coeff=None,
        capture_coeff=None,
        atari_delete_coeff=None,
        atari_escape_coeff=None,
        limit_atari=None,
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
        num_games=100,
        stats_output='stats.txt'
    ):
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
    if pattern_coeff!=None:
        command+=" --pattern_coeff " + str(pattern_coeff)
    if capture_coeff!=None:
        command+=" --capture_coeff " + str(capture_coeff)
    if atari_delete_coeff!=None:
        command+=" --atari_delete_coeff " + str(atari_delete_coeff)
    if atari_escape_coeff!=None:
        command+=" --atari_escape_coeff " + str(atari_escape_coeff)
    if limit_atari!=None:
        command+=" --limit_atari " + str(limit_atari)
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
    
    os.environ[prog]=command

    twogtp= 'gogui-twogtp'
    twogtp+=' -black "$BLACK"'
    twogtp+=' -white "$WHITE"'
    twogtp+=' -games '+ str(num_games)
    twogtp+=' -size ' + str(size)
    twogtp+=' -komi ' + str(komi)
    twogtp+=' -auto -sgffile ./stats/stats'

    if os.path.isfile('./stats/stats.dat'):
       subprocess.Popen("rm ./stats/stats*",shell=True)

    p=subprocess.Popen(twogtp,shell=True)
    p.wait()

    if p.returncode == 0 :
      (wrate,total) = winning_rate(prog)
      write_stats(stats_output,{
        'COL'    : prog[0],
        'WRATE'  : "{0:.4f}".format(wrate),
        'TOTAL'  : total, 
        'SIZE'   : size, 
        'BCOEFF' : bandit_coeff,
        'ACOEFF' : amaf_coeff,
        'FBOARD' : fill_board,
        'LGCOEF' : long_game_coeff,
        'PCF'    : pattern_coeff,
        'CCF'    : capture_coeff,
        'ADC'    : atari_delete_coeff,
        'AEC'    : atari_escape_coeff,
        'LAT'    : limit_atari,
        'CYCLES' : cycles_mcts,
        'THREADS': threads_mcts,
        'LEXP'   : limit_expansion,
        'KOMI'   : komi,
        'ROOT_P' : root_parallel,
        'CHIN_R' : chinese_rules,
        'TRAND'  : totally_random,
        'RAVE'   : rave,
        'PATT'   : patterns
      })
    else:
      write_stats(stats_output,{
        'COL'   : prog[0],
        'WRATE' : 'ERROR:',
        'TOTAL' : 'COD'+str(p.returncode)
      })
