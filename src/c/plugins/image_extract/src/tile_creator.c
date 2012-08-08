#include "image_extract.h"

void set_grayscale(unsigned char *ptr, float val)
{
    ptr[0] = (unsigned char) ((int) val & 0xFF);
    ptr[1] = (unsigned char) ((int) val >> 8);
}

void get_width_height(int *height, int *width, int current_dimension,
        t_vol *volume)
{
    if (current_dimension == X) {
        *height = volume->size[Y];
        *width = volume->size[Z];
    } else if (current_dimension == Y) {
        *height = volume->size[X];
        *width = volume->size[Z];
    } else {
        *height = volume->size[X];
        *width = volume->size[Y];
    }
}

int check_pixels_range(int width, int height, int h_position, int w_position,
        int h_position_end, int w_position_end)
{
    if (h_position > height || w_position > width) {
        printf("X - Y coordinates out of range\n");
        return (1);
    }
    if (h_position_end > height || w_position_end > width) {
        printf("X - Y coordinates out of range\n");
        return (2);
    }
    if (h_position < 0 || w_position < 0 || h_position_end < 0
            || w_position_end < 0) {
        printf("Negative X - Y coordinates\n");
        return (3);
    }
    if (h_position > h_position_end || w_position > w_position_end) {
        printf("X - Y coordinates bigger than X_END - Y_END coordinates\n");
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

void print_image(char *hyperslab, t_vol *volume, int current_dimension,
        unsigned int current_slice, int width, int height, t_image_args *a)
{
    ExceptionInfo exception;
    Image *img;
    Image *tmp;
    RectangleInfo *portion;
    ImageInfo *image_info;
    int kind;
    short streamToSocket;

    if (a->requests->is_expired(a->requests, a->info->request_id, a->info->request_time)) {
  	  close(fileno(a->file));
  	  return;
    }

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

    if ((img = ConstituteImage(width, height, "I", CharPixel, hyperslab,
            &exception)) == NULL) {
        CatchException(&exception);
        DestroyImageInfo(image_info);
        fclose_check(a->file);
        return;
    }

    tmp = img;
    if ((img = FlipImage(img, &exception)) == NULL) {
        CatchException(&exception);
        DestroyImage(tmp);
        DestroyImageInfo(image_info);
        fclose_check(a->file);
        return;
    }
    DestroyImage(tmp);

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

    // write image
    if (streamToSocket) { // SOCKET STREAM
        strcpy(img->magick, a->info->image_type);
    	image_info->file = a->file;

    	WriteImage(image_info, img);

        // clean up
        DestroyImage(img);
        DestroyImageInfo(image_info);

        fclose_check(a->file);
    } else { // WRITE FILE
    	a->info->image_type = strlower(a->info->image_type);

    	if (!a->info->root_path) {
    		printf("Error: root path is NULL\n");
            return;
    	}

        char dir[200]; // first path
        printf("%s\n", a->info->root_path);
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

        printf("%s\n", img->filename);

        WriteImage(image_info, img);

        DestroyImage(img);
        DestroyImageInfo(image_info);

        free(finalPath->buffer);
        free(finalPath);
    }
}
