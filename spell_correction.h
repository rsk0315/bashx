/* spell_correction.h -- The structures used internally to fix commands,
   and the extern declarations of the functions used to create them. */

/* Copyright (C) 1993-2016 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined (SPELL_CORRECTION_H)
#define SPELL_CORRECTION_H

#define SPELL_CORRECTION 1

#include "stdc.h"
#include "command.h"

extern int needs_correction;
extern COMMAND *command_to_check;

extern void inspect_wordlist __P((WORD_LIST *, const char *, const char *));
extern void inspect_command __P((COMMAND *, int, int));

#endif  /* SPELL_CORRECTION_H */
