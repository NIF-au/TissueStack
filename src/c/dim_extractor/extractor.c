#include <magick/api.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int		main(int ac, char **av)
{
  int		fd;
  int		dim_size[] = {499, 1311, 679};
  int		size[] = {890169, 338821, 654189};
  int		offset;
  char		*buff;
  int		slice;
  int		dim;
  Image		*img;
  ExceptionInfo exception;
  ImageInfo	*image_info;
  char		name[200];
  int		tmp_off;

  ac = ac;
  av = av;

  InitializeMagick("./");
  buff = malloc(size[0] + 1 * sizeof(*buff));
  fd = open("./raw_data_file", O_RDWR);
  //  offset = (size[0] * 499) + (size[1] * 1311) + size[2] * 300;
  // lseek(fd, offset, SEEK_SET);
  //  read(fd, buff, size[2]);

  GetExceptionInfo(&exception);
  if ((image_info = CloneImageInfo(NULL)) == NULL) {
    CatchException(&exception);
    return 8;
  }

  dim = 0;
  tmp_off = 0;
  while (dim < 3)
    {
      slice = 0;
      while (slice < dim_size[dim])
	{
	  memset(buff, '\0', size[0]);
	  offset = tmp_off + (size[dim] * slice);
	  lseek(fd, offset, SEEK_SET);
	  read(fd, buff, size[dim]);
	  slice++;
	  if (dim == 0)
	    {
	      if ((img = ConstituteImage(679, 1311, "I", CharPixel, buff, &exception)) == NULL) {
		CatchException(&exception);
		return 8;
	      }
	    }
	  else if (dim == 1)
	    {
	      if ((img = ConstituteImage(679, 499, "I", CharPixel, buff, &exception)) == NULL) {
		CatchException(&exception);
		return 8;
	      }
	    }
	  else if (dim == 2)
	    {
	      if ((img = ConstituteImage(1311, 499, "I", CharPixel, buff, &exception)) == NULL) {
		CatchException(&exception);
		return 8;
	      }
	    }
	  sprintf(name, "./png/%i-%i.png", dim, slice);
	  strcpy(img->filename, name);
	  WriteImage(image_info, img);
	  DestroyImage(img);
	  printf("slice = %i\n", slice);
	}
      tmp_off += size[dim] * dim_size[dim];
      dim++;
    }


  DestroyImage(img);
  DestroyImageInfo(image_info);
  close(fd);
  return (0);
}
