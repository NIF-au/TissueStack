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
#ifndef __MINC_NIFTI_CL_CONVERTE__
#define __MINC_NIFTI_CL_CONVERTE__

#include "core.h"
#include "utils.h"
#include "nifti_converter.h"
#include "minc_converter.h"

#include <sys/stat.h>

void 			convertMinc(char * minc_path, int fd);
void 			convertNifti(char * nifti_path, int fd);

#endif /* __MINC_NIFTI_CL_CONVERTE__ */
