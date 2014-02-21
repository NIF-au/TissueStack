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
#include "core.h"

char			*_log_levels[] = {"DEBUG",
					  "INFO",
					  "WARNING",
					  "ERROR",
					  "FATAL"};

char		*concat_path(char *root, char *path, char *extension)
{
  char		*complete_path;
  int		len;

  len = strlen(root) + strlen(path) + strlen(extension) + 2;
  complete_path = malloc(len * sizeof(complete_path));
  complete_path = memset(complete_path, '\0', len);
  complete_path = strncpy(complete_path, root, strlen(root));
  if (complete_path[strlen(root) - 1] != '/')
    complete_path = strcat(complete_path, "/");
  complete_path = strcat(complete_path, path);
  complete_path = strcat(complete_path, extension);
  return (complete_path);
}

int		lc_get_str_logged(t_plugin *plugin, char *command, int log_level, t_log *log, char **log_message)
{
  int		len = 0;
  time_t	current_time;
  char		*str_time;

  current_time = time(NULL);
  str_time = ctime(&current_time);
  str_time[strlen(str_time) - 1] = '\0';
  len = asprintf(log_message, "%s | %s%c\t| %s\t| %s\n", str_time, _log_levels[log_level], (strcmp(_log_levels[log_level], "INFO") == 0 ? '\t' : ' '),
		 (plugin == NULL ? "./" : plugin->path),
		 command);
   return (len);
}

void		create_string_and_write(t_plugin *plugin, char *command, int log_level, t_log *log, int fd)
{
  char		*log_message;
  int		len = 0;

  len = lc_get_str_logged(plugin, command, log_level, log, &log_message);
  write(fd, log_message, len);
  free(log_message);
}

void		lc_write_on_plug_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t)
{
  t_log_plug_fd	*tmp;
  t_log		*log;
  char		*complete_path = NULL;

  log = t->log;
  if (!log->path)
    return;
  if ((tmp = log->first_plug_fd) == NULL)
    {
      log->first_plug_fd = malloc(sizeof(*log->first_plug_fd));
      tmp = log->first_plug_fd;
      tmp->plugin = plugin;
      if (plugin == NULL)
	complete_path = concat_path(log->path, "core", ".plugin.log");
      else
	complete_path = concat_path(log->path, plugin->name, ".plugin.log");
      if ((tmp->fd = open(complete_path, (O_RDWR | O_CREAT | O_APPEND), 0666)) == -1)
	{
	  ERROR("open %s failed", complete_path);
	  return;
	}
      if (chmod(complete_path, 0644) == -1)
	{
	  ERROR("chmod 644 %s failed", complete_path);
	  return;
	}
      tmp->next = NULL;
    }
  else
    {
      while (tmp != NULL)
	{
	  if (plugin != NULL)
	    {
	      if (tmp->plugin && tmp->plugin->id == plugin->id)
		break;
	    }
	  else
	    {
	      if (tmp->plugin == NULL)
		break;
	    }
	  tmp = tmp->next;
	}
      if (tmp == NULL)
	{
	  tmp = malloc(sizeof(*tmp));
	  tmp->plugin = plugindup(plugin);
	  if (plugin == NULL)
	    complete_path = concat_path(log->path, "core", ".plugin.log");
	  else
	    complete_path = concat_path(log->path, plugin->name, ".plugin.log");
	  if ((tmp->fd = open(complete_path, (O_RDWR | O_CREAT | O_APPEND), 0666)) == -1)
	    {
	      ERROR("open %s failed", complete_path);
	      return;
	    }
	  if (chmod(complete_path, 0644) == -1)
	    {
	      ERROR("chmod 644 %s failed", complete_path);
	      return;
	    }
	  tmp->next = log->first_plug_fd;
	  log->first_plug_fd = tmp;
	}
    }
  if (complete_path != NULL)
    free(complete_path);
  create_string_and_write(plugin, command, log_level, log, tmp->fd);
}

void		lc_write_on_level_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t)
{
  t_log_level_fd *tmp = NULL;
  t_log		*log = NULL;
  char		*complete_path = NULL;

  if (t == NULL || t->log == NULL) return;

  log = t->log;
  if (!log->path) return;

  if ((tmp = log->first_level_fd) == NULL)
    {
      log->first_level_fd = malloc(sizeof(*log->first_level_fd));
      tmp = log->first_level_fd;
      tmp->level = log_level;
      complete_path = concat_path(log->path, _log_levels[log_level], ".level.log");
      if ((tmp->fd = open(complete_path, (O_RDWR | O_CREAT | O_APPEND), 0666)) == -1)
	{
	  ERROR("open %s failed", complete_path);
	  return;
	}
      if (chmod(complete_path, 0644) == -1)
	{
	  ERROR("chmod 644 %s failed", complete_path);
	  return;
	}
      tmp->next = NULL;
    }
  else
    {
      while (tmp != NULL)
	{
	  if (tmp->level == log_level)
	    break;
	  tmp = tmp->next;
	}
      if (tmp == NULL)
	{
	  tmp = malloc(sizeof(*tmp));
	  tmp->level = log_level;
	  complete_path = concat_path(log->path, _log_levels[log_level], ".level.log");
	  if ((tmp->fd = open(complete_path, (O_RDWR | O_CREAT | O_APPEND), 0666)) == -1)
	    {
	      ERROR("open %s failed", complete_path);
	      return;
	    }
	  if (chmod(complete_path, 0644) == -1)
	    {
	      ERROR("chmod 644 %s failed", complete_path);
	      return;
	    }
	  tmp->next = log->first_level_fd;
	  log->first_level_fd = tmp;
	}
    }
  if (complete_path != NULL)
    free(complete_path);
  create_string_and_write(plugin, command, log_level, log, tmp->fd);
}

void		lc_write_on_general_fd(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t)
{
  char		*log_message;
  int		len = 0;

  len = lc_get_str_logged(plugin, command, log_level, t->log, &log_message);
  write(t->log->general_fd, log_message, len);
  free(log_message);
}

void		lc_write_on_file(t_plugin *plugin, char *command, int log_level, t_tissue_stack *t)
{
  t_log		*log;

  log = t->log;
  if (log->write_on_files == ON)
    {
      if (log->write_on_plug_files == ON)
	lc_write_on_plug_fd(plugin, command, log_level, t);
      if (log->write_on_level_files == ON)
	lc_write_on_level_fd(plugin, command, log_level, t);
      //lc_write_on_general_fd(plugin, command, log_level, t);
    }
}

void		lc_debug(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t)
{
  t_log			*log;
  char			*log_message;

  log = t->log;
  if (log->state == OFF || log->debug == OFF)
    return;
  lc_write_on_file(plugin, command, 0, t);
  if (log->verbose == ON)
    {
      lc_get_str_logged(plugin, command, 0, t->log, &log_message);
      printf("%s", log_message);
      free(log_message);
    }
}

void		lc_info(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t)
{
  t_log		*log;
  char			*log_message;

  log = t->log;
  if (log->state == OFF)
    return;
  lc_write_on_file(plugin, command, 1, t);
  if (log->verbose == ON)
    {
      lc_get_str_logged(plugin, command, 1, t->log, &log_message);
      printf("%s", log_message);
      free(log_message);
    }
}

void		lc_warning(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t)
{
  t_log			*log;
  char			*log_message;

  log = t->log;
  if (log->state == OFF)
    return;
  lc_write_on_file(plugin, command, 2, t);
  if (log->verbose == ON)
    {
      lc_get_str_logged(plugin, command, 2, t->log, &log_message);
      printf("%s", log_message);
      free(log_message);
    }
}

void		lc_error(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t)
{
  t_log			*log;
  char			*log_message;

  log = t->log;
  if (log->state == OFF)
    return;
  lc_write_on_file(plugin, command, 3, t);
  if (log->verbose == ON)
    {
      lc_get_str_logged(plugin, command, 3, t->log, &log_message);
      printf("%s", log_message);
      free(log_message);
    }
}

void		lc_fatal(char *name, t_plugin *plugin, char *command, void *data, t_tissue_stack *t)
{
  t_log			*log;
  char			*log_message;

  log = t->log;
  if (log->state == OFF)
    return;
  lc_write_on_file(plugin, command, 4, t);
  if (log->verbose == ON)
    {
      lc_get_str_logged(plugin, command, 4, t->log, &log_message);
      printf("%s", log_message);
      free(log_message);
    }
}


void		free_all_log(t_tissue_stack *t)
{
  t_log_plug_fd	*tmp_plug;
  t_log_level_fd *tmp_level;
  t_log_plug_fd	*tmp_plug2;
  t_log_level_fd *tmp_level2;

  free(t->log->path);
  tmp_level = t->log->first_level_fd;
  tmp_plug = t->log->first_plug_fd;
  while (tmp_plug)
    {
      close(tmp_plug->fd);
      tmp_plug2 = tmp_plug;
      tmp_plug = tmp_plug->next;
      free(tmp_plug2);
    }
  while (tmp_level)
    {
      close(tmp_level->fd);
      tmp_level2 = tmp_level;
      tmp_level = tmp_level->next;
      free(tmp_level2);
    }
  free(t->log);
}
