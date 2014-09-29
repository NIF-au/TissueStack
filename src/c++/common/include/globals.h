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
#ifndef	__GLOBALS_H__
#define __GLOBALS_H__

// GLOBAL APPLICATION (DATA) PATH
#ifndef APPLICATION_PATH
#define APPLICATION_PATH "/opt/tissuestack"
#endif
// HELPS US TO ASSEMBLE SUB-DIRECTORIES BASED ON THE ROOT PATH
#define CONCAT_APP_PATH(PATH_TO_BE_ADDED) APPLICATION_PATH "/" PATH_TO_BE_ADDED

#define DATASET_PATH CONCAT_APP_PATH("data")
#define COLORMAP_PATH CONCAT_APP_PATH("colormaps")
#define LOG_PATH CONCAT_APP_PATH("logs")
#define LABEL_LOOKUP_PATH CONCAT_APP_PATH("lookup")
#define TASKS_PATH CONCAT_APP_PATH("tasks")
#define UPLOAD_PATH CONCAT_APP_PATH("upload")

#endif	/* __GLOBALS_H__ */
