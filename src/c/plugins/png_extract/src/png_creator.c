#include "minc_extract_png.h"

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


void            print_png(double *hyperslab, t_vol *volume, int current_dimension,
                          unsigned int current_slice, int width, int height)
{
  png_infop	info_ptr;
  png_structp	png_ptr;
  png_bytep	row;
  png_text	title_text;
  FILE		*png_to_write;
  int		x;
  int		y;
  char		*name;

  name = NULL;
  name = malloc((strlen("/home/oliver/workspace/png/-.png") + 7) * sizeof(*name));
  // create the name of the future png
  sprintf(name, "/home/oliver/workspace/png/%i-%i.png", current_dimension, current_slice);
  // open the png file
  png_to_write = fopen(name, "wb");
  // create the png_ptr
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  // create a pointer for the png info
  info_ptr = png_create_info_struct(png_ptr);
  setjmp(png_jmpbuf(png_ptr));
  // init the png type and the header
  png_init_io(png_ptr, png_to_write);
  png_set_IHDR(png_ptr, info_ptr, width, height, 16,
	       PNG_COLOR_TYPE_GRAY,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE); // 16 = number of bit - ex: image 16 bit
  // create the title and set the compression level of the future png
  title_text.compression = PNG_TEXT_COMPRESSION_NONE;
  title_text.key = "Title";
  title_text.text = "Hello";
  // settings and writing title and info
  png_set_text(png_ptr, info_ptr, &title_text, 1);
  png_write_info(png_ptr, info_ptr);
  // memory allocation for a row of pixels
  row = (png_bytep)malloc(2 * width * sizeof(png_byte)); // 2 = 2 byte per pixel
  y = 0;
  // loop every pixels
  while (y < height)
    {
      x = 0;
      while (x < width)
	{
	  set_grayscale(&row[(x * 2)], (float)hyperslab[(y * width) + x]);
	  x++;
	}
      png_write_row(png_ptr, row);
      y++;
    }
  // writing and finishig all the pnglib stuff
  png_write_end(png_ptr, NULL);
  fclose(png_to_write);
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  free(row);
}
