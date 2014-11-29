/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU Go, a Go program. Contact gnugo@gnu.org, or see   *
 * http://www.gnu.org/software/gnugo/ for more information.      *
 *                                                               *
 * To facilitate development of the Go Text Protocol, the two    *
 * files gtp.c and gtp.h are licensed under less restrictive     *
 * terms than the rest of GNU Go.                                *
 *                                                               *
 * Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 and  *
 * 2009 by the Free Software Foundation.                         *
 *                                                               *
 * Permission is hereby granted, free of charge, to any person   *
 * obtaining a copy of this file gtp.c, to deal in the Software  *
 * without restriction, including without limitation the rights  *
 * to use, copy, modify, merge, publish, distribute, and/or      *
 * sell copies of the Software, and to permit persons to whom    *
 * the Software is furnished to do so, provided that the above   *
 * copyright notice(s) and this permission notice appear in all  *
 * copies of the Software and that both the above copyright      *
 * notice(s) and this permission notice appear in supporting     *
 * documentation.                                                *
 *                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY     *
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE    *
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
 * PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO      *
 * EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS  *
 * NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR    *
 * CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING    *
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF    *
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT    *
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS       *
 * SOFTWARE.                                                     *
 *                                                               *
 * Except as contained in this notice, the name of a copyright   *
 * holder shall not be used in advertising or otherwise to       *
 * promote the sale, use or other dealings in this Software      *
 * without prior written authorization of the copyright holder.  *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "gtp.h"

/* We need to keep track of the board size in order to be able to
 * convert between coordinate descriptions. We could also have passed
 * the board size in all calls needing it, but that would be
 * unnecessarily inconvenient.
 */
static int gtp_boardsize = -1;

/* Current id number. We keep track of this internally rather than
 * pass it to the functions processing the commands, since those can't
 * do anything useful with it anyway.
 */
static int current_id;

/* The file all GTP output goes to.  This is made global for the user
 * of this file may want to use functions other than gtp_printf() etc.
 * Set by gtp_main_loop().
 */
FILE *gtp_output_file = NULL;


/* Read filehandle gtp_input linewise and interpret as GTP commands. */
void
gtp_main_loop(struct gtp_command commands[],
	      FILE *gtp_input, FILE *gtp_output, FILE *gtp_dump_commands)
{
  char line[GTP_BUFSIZE];
  char command[GTP_BUFSIZE];
  char *p;
  int i;
  int n;
  int status = GTP_OK;

  gtp_output_file = gtp_output;

  while (status == GTP_OK) {
    /* Read a line from gtp_input. */
    if (!fgets(line, GTP_BUFSIZE, gtp_input))
      break; /* EOF or some error */

    if (gtp_dump_commands) {
      fputs(line, gtp_dump_commands);
      fflush(gtp_dump_commands);
    }    

    /* Preprocess the line. */
    for (i = 0, p = line; line[i]; i++) {
      char c = line[i];
      /* Convert HT (9) to SPACE (32). */
      if (c == 9)
	*p++ = 32;
      /* Remove CR (13) and all other control characters except LF (10). */
      else if ((c > 0 && c <= 9)
	       || (c >= 11 && c <= 31)
	       || c == 127)
	continue;
      /* Remove comments. */
      else if (c == '#')
	break;
      /* Keep ordinary text. */
      else
	*p++ = c;
    }
    /* Terminate string. */
    *p = 0;
	
    p = line;

    /* Look for an identification number. */
    if (sscanf(p, "%d%n", &current_id, &n) == 1)
      p += n;
    else
      current_id = -1; /* No identification number. */

    /* Look for command name. */
    if (sscanf(p, " %s %n", command, &n) < 1)
      continue; /* Whitespace only on this line, ignore. */
    p += n;

    /* Search the list of commands and call the corresponding function
     * if it's found.
     */
    for (i = 0; commands[i].name != NULL; i++) {
      if (strcmp(command, commands[i].name) == 0) {
	status = (*commands[i].function)(p);
	break;
      }
    }
    if (commands[i].name == NULL)
      gtp_failure("unknown command");

    if (status == GTP_FATAL)
      gtp_panic();
  }
}

/* Set the board size used in coordinate conversions. */
void
gtp_internal_set_boardsize(int size)
{
  gtp_boardsize = size;
}

/* This currently works exactly like printf. */
void
gtp_printf(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(gtp_output_file, format, ap);
  va_end(ap);
}


/* Write a number of space separated vertices. The moves are sorted
 * before being written.
 */
void
gtp_print_vertices(int n, INDEX movei[], INDEX movej[])
{
  int k;
  int ri, rj;
  
  assert(gtp_boardsize > 0);
  
  for (k = 0; k < n; k++) {
    if (k > 0)
      gtp_printf(" ");
    if (movei[k] == -1 && movej[k] == -1)
      gtp_printf("PASS");
    else if (movei[k] < 0 || movei[k] >= gtp_boardsize
	     || movej[k] < 0 || movej[k] >= gtp_boardsize)
      gtp_printf("??");
    else {
	  ri = movei[k];
	  rj = movej[k];
      gtp_printf("%c%d", 'A' + rj + (rj >= 8), ri+1);
    }
  }
}

/* Write a single move. */
void
gtp_print_vertex(INDEX i, INDEX j)
{
  gtp_print_vertices(1, &i, &j);
}

/* Write success or failure indication plus identity number if one was
 * given.
 */
void
gtp_start_response(int status)
{
  if (status == GTP_SUCCESS)
    gtp_printf("=");
  else
    gtp_printf("?");
  
  if (current_id < 0)
    gtp_printf(" ");
  else
    gtp_printf("%d ", current_id);
}


/* Finish a GTP response by writing a double newline and returning GTP_OK. */
int
gtp_finish_response()
{
  gtp_printf("\n\n");
  return GTP_OK;
}


/* Write a full success response. Except for the id number, the call
 * is just like one to printf.
 */
int
gtp_success(const char *format, ...)
{
  va_list ap;
  gtp_start_response(GTP_SUCCESS);
  va_start(ap, format);
  vfprintf(gtp_output_file, format, ap);
  va_end(ap);
  return gtp_finish_response();
}


/* Write a full failure response. The call is identical to gtp_success. */
int
gtp_failure(const char *format, ...)
{
  va_list ap;
  gtp_start_response(GTP_FAILURE);
  va_start(ap, format);
  vfprintf(gtp_output_file, format, ap);
  va_end(ap);
  return gtp_finish_response();
}


/* Write a panic message. */
void
gtp_panic()
{
  gtp_printf("! panic\n\n");
}


/* Convert a string describing a color, "b", "black", "w", or "white",
 * to GNU Go's integer representation of colors. Return the number of
 * characters read from the string s.
 */
int
gtp_decode_color(char *s, Player *color)
{
  char color_string[7];
  int i;
  int n;

  assert(gtp_boardsize > 0);

  if (sscanf(s, "%6s%n", color_string, &n) != 1)
    return 0;

  for (i = 0; i < (int) strlen(color_string); i++)
    color_string[i] = tolower((int) color_string[i]);

  if (strcmp(color_string, "b") == 0
      || strcmp(color_string, "black") == 0)
    *color = Black;
  else if (strcmp(color_string, "w") == 0
	   || strcmp(color_string, "white") == 0)
    *color = White;
  else
    return 0;
  
  return n;
}


/* Convert an intersection given by a string to two coordinates
 * according to GNU Go's convention. Return the number of characters
 * read from the string s.
 */
int
gtp_decode_coord(char *s, INDEX *i, INDEX *j)
{
  char column;
  int row;
  int n;

  assert(gtp_boardsize > 0);

  if (sscanf(s, " %c%d%n", &column, &row, &n) != 2)
    return 0;
  
  if (tolower((int) column) == 'i')
    return 0;
  *j = tolower((int) column) - 'a';
  if (tolower((int) column) > 'i')
    --*j;

  *i = row-1;

  if (*i < 0 || *i >= gtp_boardsize || *j < 0 || *j >= gtp_boardsize)
    return 0;

  return n;
}

/* Convert a move, i.e. "b" or "w" followed by a vertex to a color and
 * coordinates. Return the number of characters read from the string
 * s. The vertex may be "pass" and then the coordinates are set to (-1, -1).
 */
int
gtp_decode_move(char *s, DataGo* move)
{
  int n1, n2;
  int k;
  Player color;
  INDEX i,j;

  assert(gtp_boardsize > 0);

  n1 = gtp_decode_color(s, &color);
  if (n1 == 0)
    return 0;

  n2 = gtp_decode_coord(s + n1, &i, &j);
  if (n2 == 0) {
    char buf[6];
    if (sscanf(s + n1, "%5s%n", buf, &n2) != 1)
      return 0;
    for (k = 0; k < (int) strlen(buf); k++)
      buf[k] = tolower((int) buf[k]);
    if (strcmp(buf, "pass") != 0)
      return 0;
    *move = PASS(color);
  }
  else
    *move = MOVE(color,i,j);
  
  return n1 + n2;
}

