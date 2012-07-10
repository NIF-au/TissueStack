#include "minc_extract_png.h"
#include "math.h"

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

int		check_pixels_range(int width, int height,
				   int h_position, int w_position,
				   int h_position_end, int w_position_end)
{
  if (h_position > height || w_position > width)
    {
      printf("X - Y coordinates out of range\n");
      return (1);
    }
  if (h_position_end > height || w_position_end > width)
    {
      printf("X - Y coordinates out of range\n");
      return (2);
    }
  if (h_position < 0 || w_position < 0 ||
      h_position_end < 0 || w_position_end < 0)
    {
      printf("Negative X - Y coordinates\n");
      return (3);
    }
  if (h_position > h_position_end || w_position > w_position_end)
    {
      printf("X - Y coordinates bigger than X_END - Y_END coordinates\n");
      return (4);
    }
  return (0);
}

int		check_and_set_position(int kind, int width, int height, t_png_args *a)
{
  int		i;

  i = 2;
  while (i == 2)
    {
      if (kind == 2)
	return (0);
      if (kind == 1)
	{
	  a->info->w_position_end = a->info->w_position + a->info->square_size;
	  a->info->h_position_end = a->info->h_position + a->info->square_size;
	}
      if (a->info->w_position_end > width)
	a->info->w_position_end = width;
      if (a->info->h_position_end > height)
	a->info->h_position_end = height;
      i = check_pixels_range(width, height,
			     a->info->h_position, a->info->w_position,
			     a->info->h_position_end, a->info->w_position_end);
      printf("***** i = %i  - h = %i - w = %i - he = %i - we = %i - height = %i - width = %i\n\n\n", i, a->info->h_position, a->info->w_position, a->info->h_position_end, a->info->w_position_end, height, width);
    }
  if (i != 0)
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
  //  printf("avant = h %i - w %i\n", a->info->h_position, a->info->w_position);
  a->info->h_position *= a->info->square_size;
  a->info->w_position *= a->info->square_size;
  // printf("apres = h %i - w %i\n", a->info->h_position, a->info->w_position);
}

#include <sys/socket.h>

void		fclose_check(FILE *file)
{
  printf("Closing 1\n");
  if (file != NULL)
    {
      printf("Closing 2\n");
      //close(fileno(file));
      //fclose(file);
      //shutdown(fileno(file), 2);
      fclose(file);
    }
}

void		print_png(char *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height, t_png_args *a)
{
  ExceptionInfo	exception;
  Image		*img;
  Image		*tmp;
  RectangleInfo *portion;
  ImageInfo	*image_info;
  int		kind;

  kind = set_service_type(a);
  convert_tiles_to_pixel_coord(a);
  //  pthread_mutex_lock(&a->info->mut);
  if (a->info->done == 1)
    return;
  if (check_and_set_position(kind, width, height, a))
    {
      fclose_check(a->file);
      return;
    }
  a->info->done = 1;
  portion = create_rectangle_crop(kind, a);
  //pthread_mutex_unlock(&a->info->mut);
  //  InitializeMagick("./");
  GetExceptionInfo(&exception);
  if ((image_info = CloneImageInfo(NULL)) == NULL)
    {
      CatchException(&exception);
      fclose_check(a->file);
      return;
    }
  if ((img = ConstituteImage(width, height, "I", CharPixel, hyperslab, &exception)) == NULL)
    {
      CatchException(&exception);
      fclose_check(a->file);
      return;
    }

  tmp = img;
  if ((img = FlipImage(img, &exception)) == NULL)
    {
      CatchException(&exception);
      fclose_check(a->file);
      return;
    }
  DestroyImage(tmp);

  strcpy(img->filename, "/home/oliver/hello.png");
  //img = SampleImage(img, 170, 328, &exception);

  if (a->info->quality != 1)
    {
      tmp = img;
      if ((img = SampleImage(img, width / a->info->quality, height / a->info->quality, &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose_check(a->file);
	  return;
	}
      DestroyImage(tmp);
      tmp = img;
      if ((img = SampleImage(img, width, height, &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose_check(a->file);
	  return;
	}
      DestroyImage(tmp);
    }
  if (a->info->scale != 1)
    {
      tmp = img;
      if ((img = ScaleImage(img, (width * a->info->scale), (height * a->info->scale), &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose_check(a->file);
	  return;
	}
      DestroyImage(tmp);
    }
  if (kind == 1 || kind == 3)
    {
      tmp = img;
      if ((img = CropImage(img, portion, &exception)) == NULL)
	{
	  CatchException(&exception);
	  fclose_check(a->file);
	  return;
	}
      DestroyImage(tmp);
    }


  ///////////////////////////////////// TEST



  /////////////////////////////////////


  if (a->file)
    {
      image_info->file = a->file;
      WriteImage(image_info, img);
    }
  else
    WriteImage(image_info, img);
  //fclose(a->file);
  //  if (ftell(a->file) != -1)
  fclose_check(a->file);

  DestroyImage(img);
  DestroyImageInfo(image_info);
  //  DestroyMagick();
}
