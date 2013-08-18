/**
  @file CheckInput.c
  This file provides a system specific implementation of CheckInput.
  It based upon code from Crafty 20.1 file "utility.c"

  Copyright (C) 2008 Peer Sommerlund

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as 
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
*/

#include "CheckInput.h"

#include "config.h"

#if defined(__WINDOWS__) || defined(WINNT)
#define NT_i386
#endif

#ifdef HAVE_SELECT
#define UNIX
#endif

#define xboard (1)
#define ics (0)
#define batch_mode (0)

#if defined(AMIGA)
#  include <proto/dos.h>
#  define tv_sec tv_secs
#  define tv_usec tv_micro
#  include <exec/types.h>
#  define RAW 1
#  define CON 0
#  include <limits.h>

int _kbhit(void)
{
  BPTR inp;
  BOOLEAN ret;

  inp = Input();
  if (!IsInteractive(inp))
    return FALSE;
  Flush(inp);
  (void) SetMode(inp, RAW);
  ret = WaitForChar(inp, 1);
  (void) SetMode(inp, CON);
  return ret;
}
/*#endif                          / * if defined(AMIGA)  * /

#if defined(NT_i386)
*/
#elif defined(NT_i386)
#  include <windows.h>
#  include <conio.h>
int CheckInput(void)
{
  int i;
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

 /*
  if (strchr(cmd_buffer, '\n'))
    return (1);
  */
  if (xboard) {
#  if defined(FILE_CNT)
    if (stdin->_cnt > 0)
      return stdin->_cnt;
#  endif
    if (!init) {
      init = 1;
      inh = GetStdHandle(STD_INPUT_HANDLE);
      pipe = !GetConsoleMode(inh, &dw);
      if (!pipe) {
        SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(inh);
      }
    }
    if (pipe) {
      if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) {
        return 1;
      }
      return dw;
    } else {
      GetNumberOfConsoleInputEvents(inh, &dw);
      return dw <= 1 ? 0 : dw;
    }
  } else {
    i = _kbhit();
  }
  return (i);
}
/*#endif

#if defined(UNIX)*/
#elif defined(UNIX)
#  ifdef __EMX__
int CheckInput(void)
{
  static KBDKEYINFO keyinfo;
  int i;

  if (!xboard && !ics && !isatty(fileno(stdin)))
    return (0);
  if (strchr(cmd_buffer, '\n'))
    return (1);
  KbdPeek(&keyinfo, 0);
  if (keyinfo.fbStatus & KBDTRF_FINAL_CHAR_IN)
    i = 1;
  else
    i = 0;
  return (i);
}
#  else
#include <string.h>
#include <stdio.h>
#include <sys/select.h>
int CheckInput(void)
{
  fd_set readfds;
  struct timeval tv;
  int data;

  if (!xboard && !ics && !isatty(fileno(stdin)))
    return (0);
  if (batch_mode)
    return (0);
  /*
  if (strchr(cmd_buffer, '\n'))
    return (1);
  */
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
#    if defined(DGT)
  if (DGT_active)
    FD_SET(from_dgt, &readfds);
#    endif
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);
  data = FD_ISSET(fileno(stdin), &readfds);
#    if defined(DGT)
  if (DGT_active)
    data |= FD_ISSET(from_dgt, &readfds);
#    endif
  return (data);
}
#  endif
#else
#error No known operating system has been defined
#endif
