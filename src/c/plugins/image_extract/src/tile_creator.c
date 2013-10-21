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
#include "image_extract.h"

void set_grayscale(unsigned char *ptr, float val)
{
    ptr[0] = (unsigned char) ((int) val & 0xFF);
    ptr[1] = (unsigned char) ((int) val >> 8);
}

int check_pixels_range(int width, int height, int h_position, int w_position,
        int h_position_end, int w_position_end)
{
    if (h_position > height || w_position > width) {
        ERROR("X - Y coordinates out of range");
        return (1);
    }
    if (h_position_end > height || w_position_end > width) {
        ERROR("X - Y coordinates out of range");
        return (2);
    }
    if (h_position < 0 || w_position < 0 || h_position_end < 0
            || w_position_end < 0) {
        ERROR("Negative X - Y coordinates");
        return (3);
    }
    if (h_position > h_position_end || w_position > w_position_end) {
        ERROR("X - Y coordinates bigger than X_END - Y_END coordinates");
        return (4);
    }
    return (0);
}

int check_and_set_position(int kind, int width, int height, t_image_args *a)
{
    int i;

    width *= a->info->scale;
    height *= a->info->scale;
    i = 2;
    while (i == 2) {
    	if (a->info->w_position > width) {
    		a->info->w_position = a->info->start_w;
    	}
    	if (a->info->h_position > height) {
    		a->info->h_position = a->info->start_h;
    	}
        if (kind == 2)
            return (0);
        if (kind == 1) {
            a->info->w_position_end = a->info->w_position
                    + a->info->square_size;
            a->info->h_position_end = a->info->h_position
                    + a->info->square_size;
        }
        if (a->info->w_position_end > width)
            a->info->w_position_end = width;
        if (a->info->h_position_end > height)
            a->info->h_position_end = height;
        i = check_pixels_range(width, height, a->info->h_position,
                a->info->w_position, a->info->h_position_end,
                a->info->w_position_end);
    }
    if (i != 0)
        return (1);
    return (0);
}

int set_service_type(t_image_args * a)
{
    if (strcmp(a->info->service, "tiles") == 0)
        return (1);
    if (strcmp(a->info->service, "full") == 0)
        return (2);
    if (strcmp(a->info->service, "images") == 0)
        return (3);
    return (0);
}

RectangleInfo *create_rectangle_crop(int kind, t_image_args *a)
{
    RectangleInfo *portion;

    if (kind == 1 || kind == 3) {
        portion = malloc(sizeof(*portion));
        portion->width = (
			  kind == 1 ?
			  a->info->square_size :
			  a->info->w_position_end - a->info->w_position);
        portion->height = (
			   kind == 1 ?
			   a->info->square_size :
			   a->info->h_position_end - a->info->h_position);
        portion->x = a->info->w_position;
        portion->y = a->info->h_position;
        return (portion);
    }
    return (NULL);
}

void convert_tiles_to_pixel_coord(t_image_args *a)
{
    a->info->h_position *= a->info->square_size;
    a->info->w_position *= a->info->square_size;
}

void fclose_check(FILE *file) {
	if (file && fcntl(fileno(file), F_GETFL) != -1) {
        fclose(file);
    }
}

void		apply_colormap(PixelPacket *px, PixelPacket *px_final, float **premapped_colormap,
			       t_image_args *a, int width, int height, unsigned long quantum_depth)
{
  int		i = 0;
  int		j = 0;
  unsigned long long int pixel_value;

  while (i < height)
    {
      j = 0;
      while (j < width)
	{
	  pixel_value = (unsigned long long int) px[(width * i) + j].red;
	  if (QuantumDepth != 8 && quantum_depth == QuantumDepth) pixel_value = mapUnsignedValue(quantum_depth, 8, pixel_value);

	  pixel_value = (pixel_value >= 255 ? 255 : pixel_value);

	  px_final[(width * i) + j].red = (unsigned char)(premapped_colormap[pixel_value][0]);
	  px_final[(width * i) + j].green = (unsigned char)(premapped_colormap[pixel_value][1]);
	  px_final[(width * i) + j].blue = (unsigned char)(premapped_colormap[pixel_value][2]);

	  if (QuantumDepth == 16 && quantum_depth == QuantumDepth) { // graphicmagick quantum depth mess which we have to react to at runtime
		  px_final[(width * i) + j].red =
				  (unsigned short)mapUnsignedValue(8, 16, (unsigned long long) premapped_colormap[pixel_value][0]);
		  px_final[(width * i) + j].green =
				  (unsigned short)mapUnsignedValue(8, 16, (unsigned long long) premapped_colormap[pixel_value][1]);
		  px_final[(width * i) + j].blue =
				  (unsigned short)mapUnsignedValue(8, 16, (unsigned long long) premapped_colormap[pixel_value][2]);
	  } else if (QuantumDepth == 32 && quantum_depth == QuantumDepth) {
		  px_final[(width * i) + j].red =
				  (unsigned int)mapUnsignedValue(8, 32, (unsigned long long) premapped_colormap[pixel_value][0]);
		  px_final[(width * i) + j].green =
				  (unsigned int)mapUnsignedValue(8, 32, (unsigned long long) premapped_colormap[pixel_value][1]);
		  px_final[(width * i) + j].blue =
				  (unsigned int)mapUnsignedValue(8, 32, (unsigned long long) premapped_colormap[pixel_value][2]);
	  }
	  j++;
	}
      i++;
    }
}

unsigned long long int get_contrasted_value(unsigned char min, unsigned char max,
				     unsigned char dataset_min, unsigned char dataset_max, unsigned long long int initial_value)
{
  float contrast_min = (float) min;
  float contrast_max = (float) max;
  float val = (float) initial_value;

  if (val <= contrast_min) return dataset_min;
  if (val >= contrast_max) return dataset_max;

  return (unsigned long long int) lround(((val - contrast_min) / (contrast_max - contrast_min)) * (float) (dataset_max - dataset_min));
}

void		apply_contrast(PixelPacket *px, unsigned char min, unsigned char max,
			       unsigned char dataset_min, unsigned char dataset_max,
			       t_image_args *a, int width, int height, unsigned long quantum_depth)
{
  int		i = 0;
  int		j = 0;
  unsigned long long int pixel_value;

  //INFO("%u - %lu\n", QuantumDepth, quantum_depth);
  while (i < height)
    {
      j = 0;
      while (j < width)
	{
	  pixel_value = (unsigned long long int) px[(width * i) + j].red;
	  if (QuantumDepth != 8 && quantum_depth == QuantumDepth) pixel_value = mapUnsignedValue(quantum_depth, 8, pixel_value);

	  pixel_value = (pixel_value > 255 ? 255 : pixel_value);
	  pixel_value = get_contrasted_value(min, max, dataset_min, dataset_max, pixel_value);

	  // graphicmagick quantum depth mess which we have to react to at runtime
	  px[(width * i) + j].red = px[(width * i) + j].green = px[(width * i) + j].blue = (unsigned char) pixel_value;
	  if (QuantumDepth == 16 && quantum_depth == QuantumDepth) {
		  px[(width * i) + j].red = px[(width * i) + j].green = px[(width * i) + j].blue =
				  (unsigned short) mapUnsignedValue(8, 16, pixel_value);
	  } else if (QuantumDepth == 32 && quantum_depth == QuantumDepth) {
		  px[(width * i) + j].red = px[(width * i) + j].green = px[(width * i) + j].blue =
				  (unsigned int) mapUnsignedValue(8, 32, pixel_value);
	  }
	  j++;
	}
      i++;
    }
}

void		print_image(char *hyperslab, t_vol *volume, int current_dimension,
			    unsigned int current_slice, int width, int height, t_image_args *a)
{
  ExceptionInfo exception;
  Image		*img;
  Image		*tmp;
  RectangleInfo *portion;
  ImageInfo	*image_info;
  int		kind;
  short		streamToSocket;
  Image		*new_image;
  ImageInfo	*new_image_info;
  ImageInfo	*image_info_cpy;
  PixelPacket	*px;
  PixelPacket	*px_tmp;

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (a->info->percentage == 1 && strcmp(a->info->service, "full") == 0)
      a->general_info->percent_add(1, a->info->id_percent, a->general_info);

  streamToSocket = a->file && fcntl(fileno(a->file), F_GETFL) != -1;

  kind = set_service_type(a);

  convert_tiles_to_pixel_coord(a);

  if (check_and_set_position(kind, width, height, a)) {
    fclose_check(a->file);
    return;
  }

  portion = create_rectangle_crop(kind, a);

  GetExceptionInfo(&exception);
  if ((image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
    CatchException(&exception);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
    return;
  }

  if (volume->original_format == MINC &&
	  ((volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'x' && volume->dim_name_char[current_dimension] == 'x') ||
      (volume->dim_name_char[0] == 'z' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[current_dimension] == 'z') ||
      (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && (volume->dim_name_char[current_dimension] == 'z' || volume->dim_name_char[current_dimension] == 'y')) ||
      (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'y' && volume->dim_name_char[2] == 'z') ||
      (volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'z' && (volume->dim_name_char[current_dimension] == 'y' || volume->dim_name_char[current_dimension] == 'x'))))
    {
      if ((img = ConstituteImage(height, width, "I", CharPixel, hyperslab, &exception)) == NULL) {
		CatchException(&exception);
		DestroyImageInfo(image_info);
		fclose_check(a->file);
		return;
      }

      if ((volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' &&
	   (volume->dim_name_char[current_dimension] == 'z' || volume->dim_name_char[current_dimension] == 'y'))) {
		  tmp = img;
		  if ((img = RotateImage(img, 90, &exception)) == NULL) {
			CatchException(&exception);
			DestroyImage(tmp);
			DestroyImageInfo(image_info);
			fclose_check(a->file);
			return;
		  }
		  DestroyImage(tmp);
      } else {
    	  tmp = img;
    	  if ((img = RotateImage(img, -90, &exception)) == NULL) {
			CatchException(&exception);
			DestroyImage(tmp);
			DestroyImageInfo(image_info);
			fclose_check(a->file);
			return;
    	  }
    	  DestroyImage(tmp);
      }
    } else {
      if ((img = ConstituteImage(width, height, "I", CharPixel, hyperslab, &exception)) == NULL) {
		CatchException(&exception);
		DestroyImageInfo(image_info);
		fclose_check(a->file);
		return;
      }
  }

	if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
		DestroyImage(img);
		DestroyImageInfo(image_info);
		write_http_header(a->file, "408 Request Timeout", a->info->image_type);
		fclose_check(a->file);
		return;
	}

  if (a->info->contrast != 0) {
      if ((px = GetImagePixelsEx(img, 0, 0, width, height, &exception)) == NULL)	{
		DestroyImage(img);
		DestroyImageInfo(image_info);
		fclose_check(a->file);
		CatchException(&exception);
		return;
	}

      apply_contrast(px, a->info->contrast_min, a->info->contrast_max,
		     a->volume->color_range_min, a->volume->color_range_max, a, width, height, img->depth);

      SyncImagePixels(img);
    }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
  }

  if (a->info->colormap_id > -1) {
      if ((new_image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
		CatchException(&exception);
		DestroyImageInfo(image_info);
		DestroyImageInfo(new_image_info);
		DestroyImage(img);
		fclose_check(a->file);
		return;
      }

      new_image_info->colorspace = RGBColorspace;
      new_image = AllocateImage(new_image_info);
      new_image->rows = height;
      new_image->columns = width;

      if ((px = GetImagePixelsEx(img, 0, 0, width, height, &exception)) == NULL) {
	  DestroyImage(img);
	  DestroyImageInfo(image_info);
	  DestroyImageInfo(new_image_info);
	  fclose_check(a->file);
	  CatchException(&exception);
	  return;
	}

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
		DestroyImage(img);
		DestroyImageInfo(image_info);
		DestroyImageInfo(new_image_info);
		write_http_header(a->file, "408 Request Timeout", a->info->image_type);
		fclose_check(a->file);
		return;
      }

      if ((px_tmp = SetImagePixelsEx(new_image, 0, 0, width, height, &exception)) == NULL)
	{
	  DestroyImage(img);
	  DestroyImageInfo(image_info);
	  DestroyImageInfo(new_image_info);
	  fclose_check(a->file);
	  CatchException(&exception);
	  return;
	}

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(new_image_info);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

   	  apply_colormap(px, px_tmp, a->info->premapped_colormap[a->info->colormap_id], a, width, height, img->depth);

      SyncImagePixels(new_image);

      DestroyImage(img);
      image_info_cpy = image_info;
      image_info = new_image_info;
      DestroyImageInfo(image_info_cpy);
      img = new_image;
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if ((volume->original_format != MINC) ||  (volume->original_format == MINC &&
	  !((volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'x' && volume->dim_name_char[current_dimension] == 'x') ||
      (volume->dim_name_char[0] == 'z' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[current_dimension] == 'z') ||
      (volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'z' && (volume->dim_name_char[current_dimension] == 'y' ||
      volume->dim_name_char[current_dimension] == 'x')) || (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'y' && volume->dim_name_char[2] == 'z'))))
    {
      tmp = img;
      if ((img = FlipImage(img, &exception)) == NULL) {
		CatchException(&exception);
		DestroyImage(tmp);
		DestroyImageInfo(image_info);
		fclose_check(a->file);
		return;
      }
      DestroyImage(tmp);
    }

  	 if ((volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[current_dimension] == 'z') ||
  		(volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[current_dimension] == 'y'))
    {
      tmp = img;
      if ((img = FlopImage(img, &exception)) == NULL) {
		CatchException(&exception);
		DestroyImage(tmp);
		DestroyImageInfo(image_info);
		fclose_check(a->file);
		return;
      }
      DestroyImage(tmp);
    }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (a->info->quality != 1) {
    tmp = img;
    if ((img = SampleImage(img, width / a->info->quality,
			   height / a->info->quality, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
    tmp = img;
    if ((img = SampleImage(img, width, height, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (a->info->scale != 1) {
    tmp = img;
    if ((img = ScaleImage(img, (width * a->info->scale),
			  (height * a->info->scale), &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (kind == 1 || kind == 3) {
    tmp = img;
    if ((img = CropImage(img, portion, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  // write image
  if (streamToSocket) {// && a->info->percentage == 0) { // SOCKET STREAM
    strcpy(img->magick, a->info->image_type);
    image_info->file = a->file;
    write_http_header(a->file, "200 OK", a->info->image_type);
    WriteImage(image_info, img);

    // clean up
    DestroyImage(img);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
  } else { // WRITE FILE
      a->info->image_type = strlower(a->info->image_type);

      if (!a->info->root_path) {
    	  ERROR("Error: root path is NULL");
    	  return;
      }

      char dir[200]; // first path
      sprintf(dir, "%s/%c/%i", a->info->root_path, volume->dim_name[current_dimension][0], current_slice);
      t_string_buffer * finalPath = createDirectory(dir, 0777);
      if (finalPath == NULL) {
    	  return;
      }
      // complete filename
      if (strcmp(a->info->service, "full") == 0 && a->info->quality != 1) {
    	  sprintf(dir, "/%i.low.res.%s", current_slice, a->info->image_type);
      } else {
    	  sprintf(dir, "/%i_%i.%s", a->info->start_w, a->info->start_h, a->info->image_type);
      }
      finalPath = appendToBuffer(finalPath, dir);
      strcpy(img->filename, finalPath->buffer);

      if (img)
    	  WriteImage(image_info, img);

      DestroyImage(img);
      DestroyImageInfo(image_info);
      free(finalPath->buffer);
      free(finalPath);
    }
}
