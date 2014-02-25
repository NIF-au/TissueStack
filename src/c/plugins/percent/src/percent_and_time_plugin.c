/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "percent_and_time_plugin.h"

void		percent_time_write_plug(char *str, void *box) {
  DEBUG("percent = %s", str);
  if (str && box)
    write(*((int *)box), str, strlen(str));
}

void		percent_get(char *id, void *box, t_tissue_stack *t)
{
  char		pc[4096];
  char		**result = NULL;

  if (t->percent == NULL)
    return;
  if ((result = read_from_file_by_id(id, t)) != NULL)
    sprintf(pc, "%s|%s", result[3], result[0]);
  else
    sprintf(pc, "NULL");

  if (result != NULL) free_null_terminated_char_2D_array(result);

  percent_time_write_plug(pc, box);
}

void		percent_cancel(char *id, void *box, t_tissue_stack *t)
{
  percent_get(id, box, t);
  t->percent_pause(id, t);
  sleep(1);
  t->percent_cancel(id, t);
}

void		percent_pause(char *id, void *box, t_tissue_stack *t)
{
  t->percent_pause(id, t);
  percent_get(id, box, t);
}

void		percent_resume(char *id, void *box, t_tissue_stack *t)
{
  t->percent_resume(id, t);
  percent_get(id, box, t);
}

void		*init(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;
  LOG_INIT(a);
  INFO("Initialized");
  return (NULL);
}

void		*start(void *args)
{
  t_args_plug	*a;

  a = (t_args_plug *)args;

  DEBUG("Started getting: %s", a->commands[0]);
  if (strcmp(a->commands[0], "get") == 0)
    percent_get(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "cancel") == 0)
    percent_cancel(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "pause") == 0)
    percent_pause(a->commands[1], a->box, a->general_info);
  else if (strcmp(a->commands[0], "resume") == 0)
    percent_resume(a->commands[1], a->box, a->general_info);
  return (NULL);
}

void		*unload(void *args)
{
  INFO("Percent Plugin Unloaded");
  return (NULL);
}
