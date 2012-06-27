#include "minc_extract_png.h"
#include "math.h"

pthread_mutex_t	mut = PTHREAD_MUTEX_INITIALIZER;

void		my_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_uint_32 check;

  check = fwrite(data, 1, length, (FILE *)(png_ptr->io_ptr));
  if (check != length)
    fprintf(stderr, "Write Error pnglib\n");
}

void		set_grayscale(png_byte *ptr, float val)
{
  ptr[0] = (png_byte)((int)val & 0xFF);
  ptr[1] = (png_byte)((int)val >> 8);
}

void		get_width_height(int *height, int *width, int current_dimension, t_vol *volume)
{
  if (current_dimension == X)
    {
      *height = volume->size[Y];
      *width = volume->size[Z];
    }
  else if (current_dimension == Y)
    {
      *height = volume->size[X];
      *width = volume->size[Z];
    }
  else
    {
      *height = volume->size[X];
      *width = volume->size[Y];
    }
}

void		image_calculus(int width, int height, double *hyperslab, png_structp png_ptr)
{
  int		x;
  int		y;
  png_bytep	row;

  row = (png_bytep)malloc((2 * width) * sizeof(png_byte)); // 2 = 2 byte per pixel
  y = height;
  while (y >= 0)
    {
      x = 0;
      while (x < width)
	{
	  set_grayscale(&row[(x * 2)], (float)hyperslab[(y * width) + x]);
	  x++;
	}
      png_write_row(png_ptr, row);
      y--;
    }
  free(row);
}

void		image_calculus_scaled_entire(int width, int height, double *hyperslab,
					     float scale, int quality, png_structp *png_ptr)
{
  int		x_diff, y_diff;
  int		_x, _y;
  int		x, y;
  int		index;
  int		i;
  int		offset;
  int		gray;
  int		a, b, c, d;
  float		x_ratio, y_ratio;
  png_bytep	row;
  int		max;
  int		row_done;
  float scale_save;

  //  pthread_mutex_lock(&mut);
  scale_save = scale;
  if (scale < 0 || quality < 0 || height < 0 || width < 0 || hyperslab == NULL || png_ptr == NULL)
    return;
  printf("FULL width = %i - height = %i - scale = %f - quality = %i\n\n", width, height, scale, quality);
  row_done = 0;
  max = width * height;
  row = (png_bytep)malloc((3 * (width * scale)) * sizeof(png_byte)); // 2 = 2 byte per pixel
  scale = scale / quality;
  x_ratio = ((float)(width - 1) / (width * scale));
  y_ratio = ((float)(height - 1) / (height * scale));
  y = (height * scale) - 1;
  while (y >= 0)//height * scale)
    {
      x = 0;
      offset = 0;
      while (x < (width * scale))
	{
	  _x = (int)(x_ratio * x);
	  _y = (int)(y_ratio * y);
	  x_diff = (x_ratio * x) - _x;
	  y_diff = (y_ratio * y) - _y;
	  index = _y * width + _x;
	  if (index < max)
	    a = (int)hyperslab[index] & 0xff;
	  else
	    a = 0;
	  if (index < max)
	    b = (int)hyperslab[index + 1] & 0xff;
	  else
	    b = 0;
	  if (index + width < max)
	    c = (int)hyperslab[index + width] & 0xff;
	  else
	    c = 0;
	  if ((index + width + 1) < max)
	    d = (int)hyperslab[index + width + 1] & 0xff;
	  else
	    d = 0;
	  gray = (int)(a * (1 - x_diff) * (1 - y_diff) + b * x_diff * (1 - y_diff) +
		       c * y_diff * (1 - x_diff) + d * (x_diff * y_diff));
	  i = 0;
	  while (i < quality)
	    {
	      set_grayscale(&row[(offset * 2)], gray);
	      offset++;
	      i++;
	    }
	  x++;
	}
      i = 0;
      while (i < quality)
	{
	  if (row_done < height)
	    {
	      png_write_row(*png_ptr, row);
	    }
	  row_done++;
	  i++;
	}
      y--;
    }
  printf("//////// height * scale = %i - row_done = %i\n", (int)(height * scale_save), row_done);
  free(row);
  // pthread_mutex_unlock(&mut);
}

int		image_calculus_scaled_chunk(int width, int height, double *hyperslab,
						   float scale, int quality, png_structp *png_ptr,
						   int h_position, int w_position,
						   int h_position_end, int w_position_end)
{
  double	*my_hyperslab;

  if ((my_hyperslab = image_calculus_scaled_chunk_data(width, height, hyperslab, scale, 1, png_ptr,
						       h_position, w_position, h_position_end, w_position_end)) == NULL)
    return (-1);
  image_calculus_scaled_entire(w_position_end - w_position, h_position_end - h_position, my_hyperslab, 1, quality, png_ptr);
  return (0);
}

double		*image_calculus_scaled_chunk_data(int width, int height, double *hyperslab,
						  float scale, int quality, png_structp *png_ptr,
						  int h_position, int w_position,
						  int h_position_end, int w_position_end)
{
  int		x_diff, y_diff;
  int		_x, _y;
  int		x, y;
  int		index;
  int		offset;
  int		gray;
  int		a, b, c, d;
  float		x_ratio, y_ratio;
  double	*px;
  int		real_w;
  int		real_h;
  int		tmp;
  int		offset_max;
  int		max;

  tmp = (int)((height * scale) - h_position_end);
  if (tmp < 0)
    return (NULL);
  real_w = (w_position_end - w_position);
  real_h = (h_position_end - h_position);
  max = width * height;
  offset_max = real_h * real_w;
  offset = 0;//(real_h * real_w) - real_w;//(square_size * square_size) - square_size;
  px = malloc((real_h * real_w) * sizeof(*px));
  x_ratio = ((float)(width - 1) / (width * scale));
  y_ratio = ((float)(height - 1) / (height * scale));
  y = tmp;//(height * scale) - h_position;
  while (y < (int)(height * scale) - h_position)//tmp + real_h)//((height * scale) - h_position - real_h))
    {
      x = w_position;
      while (x < (w_position + real_w))
	{
	  _x = (int)(x_ratio * x);
	  _y = (int)(y_ratio * y);
	  x_diff = (x_ratio * x) - _x;
	  y_diff = (y_ratio * y) - _y;
	  index = _y * width + _x;
	  if (index < max)
	    a = (int)hyperslab[index] & 0xff;
	  else
	    a = 0;
	  if (index < max)
	    b = (int)hyperslab[index + 1] & 0xff;
	  else
	    b = 0;
	  if (index + width < max)
	    c = (int)hyperslab[index + width] & 0xff;
	  else
	    c = 0;
	  if ((index + width + 1) < max)
	    d = (int)hyperslab[index + width + 1] & 0xff;
	  else
	    d = 0;
	  gray = (int)(a * (1 - x_diff) * (1 - y_diff) + b * x_diff * (1 - y_diff) +
		       c * y_diff * (1 - x_diff) + d * (x_diff * y_diff));
	  if (offset < offset_max)
	    px[offset] = gray;
	  offset++;
	  x++;
	}
      y++;
    }
  return (px);
}

void		printf_header_png(png_structp *png_ptr, png_infop *info_ptr,
				  char *dimension_name, unsigned int  current_slice,
				  FILE *png_to_write, int width, int height,
				  float scale, int scaled)
{
  png_text	title_text;
  char		*name;
  int		w;
  int		h;

  if (png_to_write == NULL)
    return;
  if (scaled == -1)
    {
      h = height;
      w = width;
    }
  else
    {
      h = height * scale;
      w = width * scale;
    }
  setjmp(png_jmpbuf(*png_ptr));
  // init the png type and the header

  //  png_init_io(*png_ptr, png_to_write);

  png_set_write_fn(*png_ptr, png_to_write, my_write, NULL);

  png_set_IHDR(*png_ptr, *info_ptr, w, h, 16,
	       PNG_COLOR_TYPE_GRAY/*_ALPHA*/,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE); // 16 = number of bit - ex: image 16 bit
  // create the title and set the compression level of the future png
  if (png_get_valid(*png_ptr, *info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(*png_ptr);
  title_text.compression = PNG_TEXT_COMPRESSION_NONE;
  name = malloc((10/*(strlen(dimension_name)*/ + 5) * sizeof(*name));
  sprintf(name, "%s - %i", dimension_name, current_slice);
  title_text.key = "TissueStack";
  title_text.text = name;
  // settings and writing title and info
  png_set_text(*png_ptr, *info_ptr, &title_text, 1);
  png_write_info(*png_ptr, *info_ptr);
  //  free(name);
}

int		check_pixels_range(int width, int height,
				   int h_position, int w_position,
				   int h_position_end, int w_position_end)
{
  if (h_position > height || w_position > width ||
      h_position_end > height || w_position_end > width)
    {
      printf("\n\nSa Mere pk tu me dit out of range\nh_position = %i - w_position = %i - h_position_end = %i - w_position_end = %i - width = %i - height = %i\n\n\n", h_position, w_position, h_position_end, w_position_end, width, height);
      printf("X - Y coordinates out of range\n");
      return (1);
    }
  if (h_position < 0 || w_position < 0 ||
      h_position_end < 0 || w_position_end < 0)
    {
      printf("Negative X - Y coordinates\n");
      return (1);
    }
  if (h_position > h_position_end || w_position > w_position_end)
    {
      printf("X - Y coordinates bigger than X_END - Y_END coordinates\n");
      return (1);
    }
  return (0);
}

int		check_and_set_position(int kind, int width, int height, t_png_args *a)
{
  if (kind == 2)
    return (0);
  if (kind == 1)
    {
      printf("yop\n");
      a->info->w_position_end = a->info->w_position + a->info->square_size;
      a->info->h_position_end = a->info->h_position + a->info->square_size;
    }
  if (a->info->w_position_end > width)
    a->info->w_position_end = width;
  if (a->info->h_position_end > height)
    a->info->h_position_end = height;
  if (check_pixels_range(width, height,
			 a->info->h_position, a->info->w_position,
			 a->info->h_position_end, a->info->w_position_end))
    return (1);
  return (0);
}

int		set_service_type(t_png_args * a)
{
  if (strcmp(a->info->service, "tiles") == 0)
    return (1);
  if (strcmp(a->info->service, "full") == 0)
    return (2);
  if (strcmp(a->info->service, "images") == 0)
    return (3);
  return (0);
}

RectangleInfo	*create_rectangle_crop(int kind, t_png_args *a)
{
  RectangleInfo *portion;

  if (kind == 1 || kind == 3)
    {
      portion = malloc(sizeof(*portion));
      portion->width = (kind == 1 ? a->info->square_size : a->info->w_position_end - a->info->w_position);
      portion->height = (kind == 1 ? a->info->square_size : a->info->h_position_end - a->info->h_position);
      portion->x = a->info->w_position;
      portion->y = a->info->h_position;
      return (portion);
    }
  return (NULL);
}

void		convert_tiles_to_pixel_coord(t_png_args *a)
{
  printf("avant = h %i - w %i\n", a->info->h_position, a->info->w_position);
  a->info->h_position *= a->info->square_size;
  a->info->w_position *= a->info->square_size;
  printf("apres = h %i - w %i\n", a->info->h_position, a->info->w_position);
}

void		print_png(char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_png_args *a)
{
  ExceptionInfo	exception;
  Image		*img;
  RectangleInfo *portion;
  ImageInfo	*image_info;
  int		kind;

  kind = set_service_type(a);
  convert_tiles_to_pixel_coord(a);
  if (check_and_set_position(kind, width, height, a))
    return;
  portion = create_rectangle_crop(kind, a);

  InitializeMagick("./");
  GetExceptionInfo(&exception);
  if ((image_info = CloneImageInfo(NULL)) == NULL)
    {
      CatchException(&exception);
      fclose(a->file);
      return;
    }
  if ((img = ConstituteImage(width, height, "I", CharPixel, hyperslab, &exception)) == NULL)
    {
      CatchException(&exception);
      fclose(a->file);
      return;
    }
  if ((img = FlipImage(img, &exception)) == NULL)
    {
      CatchException(&exception);
      fclose(a->file);
      return;
    }
  strcpy(img->filename, "/home/oliver/hello.png");
  //img = SampleImage(img, 170, 328, &exception);

  if (a->info->scale != 1)
    {
      if ((img = ScaleImage(img, (width * a->info->scale), (height * a->info->scale), &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose(a->file);
	  return;
	}
    }
  if (kind == 1 || kind == 3)
    {
      if ((img = CropImage(img, portion, &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose(a->file);
	  return;
	}
    }

  if (a->file)
    {
      image_info->file = a->file;
      WriteImage(image_info, img);
    }
  else
    WriteImage(image_info, img);
  fclose(a->file);
}

/*
void            print_png(double *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_png_args *a)//float scale, int quality, FILE *file)
{
  png_infop	info_ptr;
  png_structp	png_ptr;
  FILE		*png_to_write;
  char		*name;
  char		*dimension_name;
  float		scale;
  int		quality;
  int		scaled;

  if (!a->file)
    return;
  if (a->info->square_size > 0 ||
      (a->info->w_position_end != -1 && a->info->h_position_end != -1))
    scaled = -1;
  else
    scaled = 1;
  scale = a->info->scale;
  quality = a->info->quality;
  dimension_name = volume->dim_name[current_dimension];
  if (a->file == NULL)
    {
      name = malloc((27 + strlen(dimension_name) + 10)  * sizeof(*name));
      // create the name of the future png
      sprintf(name, "/home/oliver/workspace/png/%s-%i-%i%%.png", dimension_name, current_slice, (int)(100 * scale));
      // open the png file
      png_to_write = fopen(name, "wb");
    }
  else
    png_to_write = a->file;
  // create the png_ptr
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  // create a pointer for the png info
  info_ptr = png_create_info_struct(png_ptr);

  if (strcmp(a->info->service, "tiles") == 0 && a->info->square_size != -1)
    {
      printf("avant service = %s - a->info->w_position = %i - square-size = %i\n\n\n\n", a->info->service, a->info->w_position, a->info->square_size);
      pthread_mutex_lock(&mut);
      a->info->h_position *= a->info->square_size;
      a->info->w_position *= a->info->square_size;
      a->info->h_position_end = ((a->info->h_position + a->info->square_size) >= height ? height : (a->info->h_position + a->info->square_size));
      a->info->w_position_end = ((a->info->w_position + a->info->square_size) >= width ? width : (a->info->w_position + a->info->square_size));
      printf("apres a->info->w_position = %i - square-size = %i\n\n\n\n", a->info->w_position, a->info->square_size);
      pthread_mutex_unlock(&mut);
      if (check_pixels_range(width, height, a->info->h_position, a->info->w_position,
			     a->info->h_position_end, a->info->w_position_end))
	{
	  fclose(png_to_write);
	  return;
	}
    }
  else if (strcmp(a->info->service, "images") == 0)
    {
      a->info->h_position_end = (a->info->h_position_end > height ? height : a->info->h_position_end);
      a->info->w_position_end = (a->info->w_position_end > width ? width : a->info->w_position_end);
      if (check_pixels_range(width, height, a->info->h_position, a->info->w_position,
			     a->info->h_position_end, a->info->w_position_end))
	{
	  fclose(png_to_write);
	  return;
	}
    }

  if (a->info->square_size > 0 || (a->info->w_position_end != -1 && a->info->h_position_end != -1))
    printf_header_png(&png_ptr, &info_ptr, dimension_name, current_slice,
		      png_to_write, (a->info->w_position_end - a->info->w_position),
		      (a->info->h_position_end - a->info->h_position), scale, scaled);
  else
    printf_header_png(&png_ptr, &info_ptr, dimension_name, current_slice,
		      png_to_write, width, height, scale, scaled);


  if (a->info->square_size == -1 && a->info->w_position_end == -1 && a->info->h_position_end == -1)
    {
      printf("Images FULL\n");
      image_calculus(width, height, hyperslab, png_ptr);
      //image_calculus_scaled_entire(width, height, hyperslab, scale, quality, &png_ptr);
    }
  if (strcmp(a->info->service, "tiles") == 0 || strcmp(a->info->service, "images") == 0)
    {
      printf("Images TILES\n");
      if (image_calculus_scaled_chunk(width, height, hyperslab, scale, quality,
				      &png_ptr, a->info->h_position, a->info->w_position,
				      a->info->h_position_end, a->info->w_position_end) == -1)
	return;
    }

  // writing and finishig all the pnglib stuff
  png_write_end(png_ptr, NULL);
  fclose(png_to_write);
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  //  free(name);
}*/
