#include "converter.h"

int		get_sign_nifti(nifti_image *nim)
{
  if (nim->datatype == 2 || nim->datatype == 512 || nim->datatype == 768)
    return (MI_PRIV_UNSIGNED);
  else
    return (MI_PRIV_SIGNED);
}

int		get_datatype_nifti(nifti_image *nim)
{
  if (nim->datatype == 2 || nim->datatype == 256)
    return(NC_CHAR);
  else if (nim->datatype == 4 || nim->datatype == 512)
    return (NC_SHORT);
  else if (nim->datatype == 8 || nim->datatype == 768)
    return (NC_INT);
  else if (nim->datatype == 16)
    return (NC_FLOAT);
  else
    return (NC_DOUBLE);
}

void		*iter_all_pix_and_convert(void *data_in, unsigned int size, nifti_image *nim)
{
  int		i;
  unsigned char	*data_out;
  double	dvalue = 0.0;
  void		*inptr;
  void		*outptr;
  int		sign;
  int		datatype;
  void		*data;

  datatype = get_datatype_nifti(nim);
  sign = get_sign_nifti(nim);



  if (nim->datatype == 2 || nim->datatype == 256) {
    if (nim->datatype == 2)
      data = (unsigned char*)data_in;
    else
      data = (char*)data_in;
  }
  else if (nim->datatype == 4 || nim->datatype == 512) {
    if (nim->datatype == 512)
      data = (unsigned short*)data_in;
    else
      data = (short*)data_in;
  }
  else if (nim->datatype == 8 || nim->datatype == 768) {
    if (nim->datatype == 768)
      data = (unsigned int*)data_in;
    else
      data = (int*)data_in;
  }
  else if (nim->datatype == 16)
    data = (float *)data_in;
  else
    data = (double*)data_in;


  data_out = malloc((size + 1) * sizeof(*data_out));
  i = 0;
  while (i < size)
    {
      if (nim->datatype == 2 || nim->datatype == 256) {
	if (nim->datatype == 2)
	  inptr = (unsigned char *)(&((unsigned char *)data)[i]);
	else
	  inptr = (char *)(&((char *)data)[i]);
      }
      else if (nim->datatype == 4 || nim->datatype == 512) {
	if (nim->datatype == 512)
	  inptr = (unsigned short *)(&((unsigned short *)data)[i]);
	else
	  inptr = (short *)(&((short *)data)[i]);
      }
      else if (nim->datatype == 8 || nim->datatype == 768) {
	if (nim->datatype == 768)
	  inptr = (unsigned int *)(&((unsigned int *)data)[i]);
	else
	  inptr = (int *)(&((int *)data)[i]);
      }
      else if (nim->datatype == 16)
	inptr = (float *)(&((float *)data)[i]);
      else
	inptr = (double *)(&((double *)data)[i]);
      //      inptr = &data[i];
      outptr = &data_out[i];
      MI_TO_DOUBLE(dvalue, datatype, sign, inptr);
      MI_FROM_DOUBLE(dvalue, NC_CHAR, MI_PRIV_UNSIGNED, outptr);
      i++;
    }
  return (data_out);
}

t_header	*create_header_from_nifti_struct(nifti_image *nifti_volume)
{
  t_header	*h;
  int		i;
  int		j;

  h = malloc(sizeof(*h));
  h->dim_nb = nifti_volume->ndim;

  h->sizes = malloc(h->dim_nb * sizeof(*h->sizes));
  h->start = malloc(h->dim_nb * sizeof(*h->start));
  h->steps = malloc(h->dim_nb * sizeof(*h->steps));
  h->dim_name = malloc(h->dim_nb * sizeof(*h->dim_name));
  h->dim_offset = malloc(h->dim_nb * sizeof(*h->dim_offset));
  h->slice_size = malloc(h->dim_nb * sizeof(*h->slice_size));

  h->slice_max = (nifti_volume->dim[1] * nifti_volume->dim[2] > nifti_volume->dim[2] * nifti_volume->dim[3] ?
		  (nifti_volume->dim[1] * nifti_volume->dim[2] > nifti_volume->dim[1] * nifti_volume->dim[3] ? nifti_volume->dim[1] * nifti_volume->dim[2] : nifti_volume->dim[1] * nifti_volume->dim[3]) :
		  (nifti_volume->dim[2] * nifti_volume->dim[3] > nifti_volume->dim[1] * nifti_volume->dim[3] ? nifti_volume->dim[2] * nifti_volume->dim[3] : nifti_volume->dim[1] * nifti_volume->dim[3]));

  i = 0;
  while (i < h->dim_nb)
    {
      h->sizes[i] = nifti_volume->dim[i + 1];
      h->start[i] = nifti_volume->sto_xyz.m[i][3];
      h->steps[i] = nifti_volume->pixdim[i + 1];
      h->dim_name[i] = strdup("xspace");
      h->dim_name[i][0] = 'x' + i;

      h->slice_size[i] = 1;
      j = 1;
      while (j < h->dim_nb + 1)
	{
	  if ((j - 1) != i)
	    h->slice_size[i] *= nifti_volume->dim[j];
	  j++;
	}
      i++;
    }

  h->dim_offset[0] = 0;
  i = 1;
  while (i < h->dim_nb)
    {
      h->dim_offset[i] = (unsigned long long)(h->dim_offset[i - 1] + (unsigned long long)((unsigned long long)h->slice_size[i - 1] * (unsigned long long)h->sizes[i - 1]));
      i++;
    }
  return (h);
}

void		write_header_into_file(int fd, t_header *h)
{
  char		head[4096];
  char		lenhead[200];
  int		len;

  memset(head, '\0', 4096);
  sprintf(head, "%i|%i:%i:%i|%g:%g:%g|%g:%g:%g|%s|%s|%s|%c|%c|%c|%i:%i:%i|%i|%llu:%llu:%llu|",
	  h->dim_nb,
	  h->sizes[0], h->sizes[1], h->sizes[2],
	  h->start[0], h->start[1], h->start[2],
	  h->steps[0], h->steps[1], h->steps[2],
	  h->dim_name[0], h->dim_name[1], h->dim_name[2],
	  h->dim_name[0][0], h->dim_name[1][0], h->dim_name[2][0],
	  h->slice_size[0], h->slice_size[1], h->slice_size[2],
	  h->slice_max,
	  (unsigned long long)h->dim_offset[0], (unsigned long long)h->dim_offset[1], (unsigned long long)h->dim_offset[2]);
  len = strlen(head);
  memset(lenhead, '\0', 200);
  sprintf(lenhead, "@IaMraW@|%i|", len);
  write(fd, lenhead, strlen(lenhead));
  write(fd, head, len);
}

void		*init(void *args)
{
  return (NULL);
}

void  		*start(void *args)
{
  int		dims[8] = { 0,  -1, -1, -1, -1, -1, -1, -1 };
  int		sizes[3];
  char		*data = NULL;
  int		slice = 0;
  int		ret;
  nifti_image	*nim;
  int		nslices;
  int		i;
  int		fd;
  char		*data_char;
  unsigned int	size_per_slice;
  t_header	*h;
  t_args_plug	*a;
  char		*id_percent;
  int		cancel = 0;
  unsigned int dimensions_resume = -1;
  unsigned int slice_resume = -1;
  unsigned long long off;

  prctl(PR_SET_NAME, "TS_NIFTI_CON");

  a = (t_args_plug*)args;
  if ((nim = nifti_image_read(a->commands[0], 0)) == NULL)
    {
      ERROR("Error Nifti read");
      return (NULL);
    }

  sizes[0] = nim->dim[1];
  sizes[1] = nim->dim[2];
  sizes[2] = nim->dim[3];

  h = create_header_from_nifti_struct(nim);

  if (a->commands[2] != NULL && a->commands[3] != NULL && a->commands[4] != NULL)
    {
      dimensions_resume = atoi(a->commands[2]);
      slice_resume = atoi(a->commands[3]);
      if ((fd = open(a->commands[1], (O_APPEND | O_RDWR))) == -1)
	{
	  ERROR("Open Failed");
	  return (NULL);
	}
      i = 0;
      while (i < dimensions_resume)
	{
	  off += h->dim_offset[i];
	  i++;
	}
      if (slice_resume != 0)
	off += h->slice_size[i] * (slice_resume - 1);
      lseek(fd, off, SEEK_SET);
      i = dimensions_resume + 1;
      id_percent = a->commands[4];
    }
  else
    {
      a->general_info->percent_init((sizes[0] + sizes[1] + sizes[2]), &id_percent, a->commands[0], "2", a->commands[1], NULL, a->general_info);
      if (a->box != NULL)
	{
	  if (write(*((int*)a->box), id_percent, 16) < 0)
	    ERROR("Open Error");
	}
      if ((fd = open(a->commands[1], O_CREAT | O_TRUNC | O_RDWR)) < 0)
	{
	  ERROR("Open error");
	  return (NULL);
	}
      if (chmod(a->commands[1], 0644) == -1)
	ERROR("Chmod Error");
      write_header_into_file(fd, h);
      i = 1;
    }
  while (i <= nim->dim[0] && cancel == 0)
    {
      if (slice_resume != -1)
	{
	  slice = slice_resume;
	  slice_resume = -1;
	}
      else
	slice = 0;
      nslices = sizes[i - 1];
      size_per_slice = h->slice_size[i - 1];
      while(slice < nslices && cancel == 0)
	{
	  data = NULL;
	  dims[i] = slice;
	  if ((ret = nifti_read_collapsed_image(nim, dims, (void*)&data)) < 0)
	    {
	      ERROR("Error Nifti Get Hyperslab");
	      return (NULL);
	    }
	  if( ret > 0 )
	    {
	      data_char = iter_all_pix_and_convert(data, size_per_slice, nim);
	      write(fd, data_char, size_per_slice);
	      free(data_char);
	    }
	  free(data);
	  slice++;
	  a->general_info->percent_add(1, id_percent, a->general_info);
	  cancel = a->general_info->is_percent_paused_cancel(id_percent, a->general_info);
	  DEBUG("Slice n %i on dimension %i slicenb = %i -- cancel = %i", slice, (i - 1), nslices, cancel);
	}
      dims[i] = -1;
      i++;
    }
  if (cancel == 0)
    INFO("Conversion: NIFTI: %s to RAW: %s ==> DONE", a->commands[0], a->commands[1]);
  if (close(fd) == -1)
    {
      ERROR("Close Error");
      return (NULL);
    }
  a->destroy(a);

  return (NULL);
}

void		*unload(void *args)
{
  return (NULL);
}
