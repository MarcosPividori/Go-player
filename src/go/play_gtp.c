
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "gtp.h"
#include "play_gtp.h"

#define DECLARE(func) static int func(char *s)

DECLARE(gtp_protocol_version);
DECLARE(gtp_name);
DECLARE(gtp_program_version);
DECLARE(gtp_known_command);
DECLARE(gtp_list_commands);
DECLARE(gtp_quit);
DECLARE(gtp_set_boardsize);
DECLARE(gtp_clear_board);
DECLARE(gtp_set_komi);
DECLARE(gtp_play);
DECLARE(gtp_genmove);
DECLARE(gtp_showboard);
DECLARE(gtp_final_score);
#ifdef DEBUG
DECLARE(gtp_debug);
DECLARE(gtp_match_patterns);
#endif

/* List of known commands. */
static struct gtp_command commands[] = {
  {"protocol_version",        gtp_protocol_version},
  {"name",                    gtp_name},
  {"version",                 gtp_program_version},
  {"known_command",    	      gtp_known_command},
  {"list_commands",    	      gtp_list_commands},
  {"quit",             	      gtp_quit},
  {"boardsize",        	      gtp_set_boardsize},
  {"clear_board",      	      gtp_clear_board},
  {"komi",        	          gtp_set_komi},
  {"play",            	      gtp_play},
  {"genmove",                 gtp_genmove},
  {"showboard",        	      gtp_showboard},
  {"final_score",             gtp_final_score},
#ifdef DEBUG
  {"debug",        	          gtp_debug},
  {"match_patterns",          gtp_match_patterns},
#endif
  {NULL,                      NULL}
};

Game *game;

/* Start playing using the Go Text Protocol. */
void
play_gtp(FILE *gtp_input, FILE *gtp_output, FILE *gtp_dump_commands, Game *g)
{
  setbuf(gtp_output, NULL);

  game = g;
  
  /* Inform the GTP utility functions about the board size. */
  gtp_internal_set_boardsize(game->get_boardsize());

  gtp_main_loop(commands, gtp_input, gtp_output, gtp_dump_commands);
}

/* Function:  Quit
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_quit(char *s)
{
  UNUSED(s);
  gtp_success("");
  return GTP_QUIT;
}

/* Function:  Report protocol version.
 * Arguments: none
 * Fails:     never
 * Returns:   protocol version number
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_protocol_version(char *s)
{
  UNUSED(s);
  return gtp_success("%d", GTP_VERSION);
}

/* Function:  Report the name of the program.
 * Arguments: none
 * Fails:     never
 * Returns:   program name
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_name(char *s)
{
  UNUSED(s);
  return gtp_success("Marcos Go");
}

/* Function:  Report the version number of the program.
 * Arguments: none
 * Fails:     never
 * Returns:   version number
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_program_version(char *s)
{
  UNUSED(s);
  return gtp_success(VERSION);
}

/* Function:  Set the board size to NxN and clear the board.
 * Arguments: integer
 * Fails:     board size outside engine's limits
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_set_boardsize(char *s)
{
  int boardsize;

  if (sscanf(s, "%d", &boardsize) < 1)
    return gtp_failure("boardsize not an integer");
  
  if (boardsize<=0 || boardsize>MAX_BOARD) {
    if (GTP_VERSION == 1)
      return gtp_failure("unacceptable boardsize");
    else
      return gtp_failure("unacceptable size");
  }

  game->set_boardsize(boardsize);
  game->clear_board();
  gtp_internal_set_boardsize(boardsize);
  return gtp_success("");
}

/* Function:  Clear the board.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_clear_board(char *s)
{
  UNUSED(s);

  game->clear_board();

  return gtp_success("");
}

/* Function:  Set the komi.
 * Arguments: float
 * Fails:     incorrect argument
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */

static int
gtp_set_komi(char *s)
{
  float komi;
  if (sscanf(s, "%f", &komi) < 1)
    return gtp_failure("komi not a float");

  game->set_komi(komi);

  return gtp_success("");
}

/* Function:  Play a stone of the given color at the given vertex.
 * Arguments: color, vertex
 * Fails:     invalid vertex, illegal move
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_play(char *s)
{
  DataGo move;

  if (!gtp_decode_move(s, &move))
    return gtp_failure("invalid color or coordinate");

  if(!game->play_move(move))
    return gtp_failure("illegal move");

  return gtp_success("");
}

/* Function:  Generate and play the supposedly best move for either color.
 * Arguments: color to move
 * Fails:     invalid color
 * Returns:   a move coordinate or "PASS" (or "resign" if resignation_allowed)
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_genmove(char *s)
{
  DataGo move;
  Player color;

  if(!gtp_decode_color(s, &color))
    return gtp_failure("invalid color");
  
  move = game->gen_move(color);
  
  if(IS_RESIGN(move))
    return gtp_success("resign");
  
  assert(game->play_move(move));
  gtp_start_response(GTP_SUCCESS);
  if(IS_PASS(move))
    gtp_print_vertex(-1,-1);
  else
    gtp_print_vertex(move.i, move.j);
  return gtp_finish_response();
}

/* Function:  Write the position to stdout.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_showboard(char *s)
{
  UNUSED(s);
  
  gtp_start_response(GTP_SUCCESS);
  gtp_printf("\n");
  game->show_board(gtp_output_file);
  return gtp_finish_response();
}

/* Function:  Compute the score of a finished game.
 * Arguments: Optional random seed
 * Fails:     never
 * Returns:   Score in SGF format (RE property).
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_final_score(char *s)
{
  float final_score=game->get_final_score();
  gtp_start_response(GTP_SUCCESS);
  if (final_score > 0.0)
    gtp_printf("W+%3.1f", final_score);
  else if (final_score < 0.0)
    gtp_printf("B+%3.1f", -final_score);
  else
    gtp_printf("0");
  return gtp_finish_response();
}

#ifdef DEBUG
/* Function:  Write the debug info to stdout.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 */
static int
gtp_debug(char *s)
{
  UNUSED(s);
  
  gtp_start_response(GTP_SUCCESS);
  gtp_printf("\n");
  game->debug();
  return gtp_finish_response();
}

/* Function:  Show debug info related to patterns to stdout.
 * Arguments: none
 * Fails:     never
 * Returns:   nothing
 *
 */
static int
gtp_match_patterns(char *s)
{
  UNUSED(s);
  
  gtp_start_response(GTP_SUCCESS);
  gtp_printf("\n");
  game->match_patterns();
  return gtp_finish_response();
}
#endif

/* Function:  List all known commands
 * Arguments: none
 * Fails:     never
 * Returns:   list of known commands, one per line
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_list_commands(char *s)
{
  int k;
  UNUSED(s);

  gtp_start_response(GTP_SUCCESS);

  for (k = 0; commands[k].name != NULL; k++)
    gtp_printf("%s\n", commands[k].name);

  gtp_printf("\n");
  return GTP_OK;
}

/* Function:  Tell whether a command is known.
 * Arguments: command name
 * Fails:     never
 * Returns:   "true" if command exists, "false" if not
 *
 * Status:    GTP version 2 standard command.
 */
static int
gtp_known_command(char *s)
{
  int k;
  char command[GTP_BUFSIZE];

  if (sscanf(s, "%s", command) == 1) {
    for (k = 0; commands[k].name != NULL; k++)
      if (strcmp(command, commands[k].name) == 0)
	return gtp_success("true");
  }

  return gtp_success("false");
}

