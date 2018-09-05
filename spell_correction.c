/* spell_correction.c -- Correct a not-found simple command. */

/* Copyright (C) 1987-2018 Free Software Foundation, Inc.

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

#include "config.h"

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>

#include "spell_correction.h"

void
repeat_output (str, count)
     const char *str;
     int count;
{
  int i;
  for (i = 0; i < count; i++)
    fprintf (stderr, "%s", str);
}

void
inspect_wordlist (words, sep, end)
     WORD_LIST *words;
     const char *sep, *end;
{
  WORD_DESC *word;

  while (words)
    {
      word = words->word;
      words = words->next;
      fprintf (stderr, "%s%s", word->word, words ? sep : end);
  }
}

void
inspect_command (command, indent, indent0)
     COMMAND *command;
     int indent, indent0;
{
  if (command == 0)
    {
      repeat_output ("  ", indent0);
      fprintf (stderr, "(nil)\n");
      return;
    }

  switch (command->type)
    {
    case cm_for:
      {
        FOR_COM *cm;
        cm = command->value.For;
        repeat_output ("  ", indent0);
        fprintf (stderr, "for: %s in {", cm->name->word);
        inspect_wordlist (cm->map_list, ", ", "}\n");
        inspect_command (cm->action, indent+1, indent+1);
      }
      break;

    case cm_case:
      {
        CASE_COM *cm;
        PATTERN_LIST *clauses;

        cm = command->value.Case;
        repeat_output ("  ", indent0);
        fprintf (stderr, "case: %s in\n", cm->word->word);
        clauses = cm->clauses;
        while (clauses)
          {
            repeat_output ("  ", indent);
            inspect_wordlist (clauses->patterns, " | ", " )\n");
            inspect_command (clauses->action, indent+1, indent+1);
            repeat_output ("  ", indent+1);
            switch (clauses->flags)
              {
              case 0:
                fprintf (stderr, ";;\n");
                break;
              case CASEPAT_FALLTHROUGH:
                fprintf (stderr, ";&\n");
                break;
              case CASEPAT_TESTNEXT:
                fprintf (stderr, ";;&\n");
                break;
              default:
                fprintf (stderr, "???\n");
              }

            clauses = clauses->next;
          }
      }
      break;

    case cm_while:
    case cm_until:
      {
        WHILE_COM *cm;

        cm = command->value.While;
        repeat_output ("  ", indent0);
        fprintf (stderr, command->type == cm_while ? "while " : "until ");
        inspect_command (cm->test, indent+2, 0);
        repeat_output ("  ", indent);
        fprintf (stderr, "do:\n");
        inspect_command (cm->action, indent+1, indent+1);
      }
      break;

    case cm_if:
      {
        IF_COM *cm;

        cm = command->value.If;
        repeat_output ("  ", indent0);
        fprintf (stderr, "if ");
        inspect_command (cm->test, indent+2, 0);
        repeat_output ("  ", indent);
        fprintf (stderr, "then:\n");
        inspect_command (cm->true_case, indent+1, indent+1);
        repeat_output ("  ", indent);
        if (cm->false_case)
          {
            fprintf (stderr, "else:\n");
            inspect_command (cm->false_case, indent+1, indent+1);
          }
      }
      break;

    case cm_simple:
      {
        WORD_LIST *words;
        WORD_DESC *word;
        SIMPLE_COM *cm;

        cm = command->value.Simple;
        words = cm->words;
        repeat_output ("  ", indent0);
        fprintf (stderr, "simple command: ");
        inspect_wordlist (words, " ", "\n");
        fflush (stderr);
      }
      break;

#if defined (SELECT_COMMAND)
    case cm_select:
      {
        SELECT_COM *cm;

        cm = command->value.Select;
        repeat_output ("  ", indent);
        fprintf (stderr, "select %s in {", cm->name->word);
        inspect_wordlist (cm->map_list, ", ", "}\n");
        repeat_output ("  ", indent);
        fprintf (stderr, "do:\n");
        inspect_command (cm->action, indent+1, indent+1);
      }
      break;
#endif

    case cm_connection:
      {
        CONNECTION *cm;

        cm = command->value.Connection;
        repeat_output ("  ", indent);
        inspect_command (cm->first, indent, indent);
        repeat_output ("  ", indent);
        switch (cm->connector)
          {
          case 38:
            fprintf (stderr, "& ");
            break;
          case 124:
            fprintf (stderr, "| ");
            break;
          case 288:
            fprintf (stderr, "&& ");
            break;
          case 289:
            fprintf (stderr, "|| ");
            break;
          default:
            fprintf (stderr, "%d ", cm->connector);
          }

        if (cm->second)
          inspect_command (cm->second, indent, indent);
        else
          fprintf (stderr, "\n");
      }

    case cm_function_def:
      {
        FUNCTION_DEF *cm;

        cm = command->value.Function_def;
        repeat_output ("  ", indent);
        fprintf (stderr, "%s ()\n", cm->name->word);
        inspect_command (cm->command, indent, indent);
      }
      break;

    case cm_group:
      {
        GROUP_COM *cm;

        cm = command->value.Group;
        repeat_output ("  ", indent0);
        fprintf (stderr, "{\n");
        inspect_command (cm->command, indent+1, indent+1);
        repeat_output ("  ", indent);
        fprintf (stderr, "}\n");
      }
      break;

#if defined (DPAREN_ARITHMETIC)
    case cm_arith:
      {
        ARITH_COM *cm;

        repeat_output ("  ", indent0);
        fprintf (stderr, "(( %s ))\n", cm->exp->word); 
      }
      break;
#endif

#if defined (COND_COMMAND)
    case cm_cond:
      {
        /* [[ ... ]] */
        COND_COM *cm, *cmc;

        cm = command->value.Cond;
        repeat_output ("  ", indent0);
        if (cm->right)
          // binary-op
          fprintf (stderr, "[[ %s %s %s ]]\n",
                   cm->left->op->word, cm->op->word, cm->right->op->word);
        else
          // binary-op
          fprintf (stderr, "[[ %s %s ]]\n",
                   cm->op->word, cm->left->op->word);
      }
      break;
#endif

#if defined (ARITH_FOR_COMMAND)
    case cm_arith_for:
      {
        ARITH_FOR_COM *cm;

        cm = command->value.ArithFor;
        repeat_output ("  ", indent0);

        fprintf (stderr, "for (( ");
        inspect_wordlist (cm->init, ", ", "; ");
        inspect_wordlist (cm->test, ", ", "; ");
        inspect_wordlist (cm->step, ", ", "))\n");

        inspect_command (cm->action, indent+1, indent+1);
      }
      break;
#endif

    case cm_subshell:
      {
        SUBSHELL_COM *cm;

        cm = command->value.Subshell;
        repeat_output ("  ", indent0);
        fprintf (stderr, "(\n");
        inspect_command (cm->command, indent+1, indent+1);
        repeat_output ("  ", indent);
        fprintf (stderr, ")\n");
      }
      break;

    case cm_coproc:
      {
        repeat_output ("  ", indent0);
        fprintf (stderr, "coproc\n");
        break;
      }

    default:
      fprintf (stderr, "unknown\n");
    }
}

/* Flags for spell correction */
int needs_correction = 0;

/* Command, if returning 127, subjected to spell correction */
COMMAND *command_to_check = 0;
