#!/usr/bin/python

import subprocess
import os.path

output = open('stats.txt','a',0)
col_sizes = [4,10,6,5,8,7,7,7,4,4,4,4,4,7,8,5,5,7,7,6,6,15]

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
    return command

def analyze_program(isWhite=False,
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
                    pattern_coeff,
                    capture_coeff,
                    atari_delete_coeff,
                    atari_escape_coeff,
                    limit_atari,
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
      write_row(output,[prog[0],
                        "{0:.4f}".format(wrate),
                        str(total),
                        str(size),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(pattern_coeff),
                        str(capture_coeff),
                        str(atari_delete_coeff),
                        str(atari_escape_coeff),
                        str(limit_atari),
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
      write_row(output,[prog[0],
                        'ERROR:',
                        'COD'+str(p.returncode),
                        str(size),
                        str(bandit_coeff),
                        str(amaf_coeff),
                        str(fill_board),
                        str(long_game_coeff),
                        str(pattern_coeff),
                        str(capture_coeff),
                        str(atari_delete_coeff),
                        str(atari_escape_coeff),
                        str(limit_atari),
                        str(cycles_mcts),
                        str(threads_mcts),
                        str(limit_expansion),
                        str(komi),
                        str(root_parallel),
                        str(chinese_rules),
                        str(totally_random),
                        str(rave),
                        str(patterns)])

write_row(output,['COL','WRATE','TOTAL','SIZE','BCOEFF','ACOEFF','FBOARD','LGCOEF','PCF','CCF','ADC','AEC','LAT','CYCLES','THREADS','LEXP','KOMI','ROOT_P','CHIN_R','TRAND','RAVE','PATT','BOOK']);

#RAVE-KNOWLEDGE vs GNU
analyze_program(isWhite=False, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=1000,
  fill_board=1,
  long_game_coeff=3,
  pattern_coeff=6,
  capture_coeff=4,
  atari_delete_coeff=4,
  atari_escape_coeff=1,
  limit_atari=24,
  cycles_mcts=30000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=False,
)

analyze_program(isWhite=False, komi=5, num_games=100,
  patterns='patterns.txt',
  bandit_coeff=0,
  amaf_coeff=2500,
  fill_board=1,
  long_game_coeff=3,
  pattern_coeff=6,
  capture_coeff=4,
  atari_delete_coeff=4,
  atari_escape_coeff=1,
  limit_atari=24,
  cycles_mcts=30000,
  threads_mcts=5,
  limit_expansion=1,
  root_parallel=False,
  chinese_rules=True,
  rave=True,
  totally_random=False,
)

output.close()
 
