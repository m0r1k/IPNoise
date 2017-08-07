#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <X11/Xlib.h>
//#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

//#include <libpng16/png.h>
#include <libpng15/png.h>
#include <jpeglib.h>

#include "file.h"

#include "snapshot.h"

int32_t write_jpeg(
    XImage      *img,
    const char  *filename)
{
    int32_t                     err         = -1;
    FILE                        *outfile    = NULL;
    char                        *buffer     = NULL;
    unsigned long               pixel;
    int                         x, y;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
    JSAMPROW                    row_pointer;

    outfile = fopen(filename, "wb");
    if (!outfile){
        PERROR("Failed to open output file %s", filename);
        goto fail;
    }

    // collect separate RGB values to a buffer
    buffer = malloc(sizeof(char)*3*img->width*img->height);
    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {
            pixel = XGetPixel(img,x,y);
            buffer[y * img->width*3 + x*3 + 0] = (char)(
                (pixel & 0xff0000) >> 16
            );
            buffer[y * img->width*3 + x*3 + 1] = (char)(
                (pixel & 0x00ff00) >> 8
            );
            buffer[y * img->width*3 + x*3 + 2] = (char)(
                (pixel & 0x0000ff) >> 0
            );
        }
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width       = img->width;
    cinfo.image_height      = img->height;
    cinfo.input_components  = 3;
    cinfo.in_color_space    = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 85, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height){
        row_pointer = (JSAMPROW) &buffer[
            cinfo.next_scanline * (img->depth >> 3)*img->width
        ];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    free(buffer);
    jpeg_finish_compress(&cinfo);
    fclose(outfile);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t write_png(
    XImage      *image,
    const char  *a_path)
{
    int32_t             err             = -1;
    FILE                *fp             = NULL;
    const char          *title          = "Screenshot";
    int32_t             width           = 0;
    int32_t             height          = 0;
    unsigned long       red_mask        = 0;
    unsigned long       green_mask      = 0;
    unsigned long       blue_mask       = 0;
    png_structp         png_ptr;
    png_infop           png_info_ptr;
    png_bytep           png_row;
    int                 x, y;

    width       = image->width;
    height      = image->height;

    red_mask    = image->red_mask;
    green_mask  = image->green_mask;
    blue_mask   = image->blue_mask;

    // Open file
    fp = fopen(a_path, "wb");
    if (!fp){
        PERROR("Could not open file for writing\n");
        goto fail;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING,
        NULL,
        NULL,
        NULL
    );
    if (!png_ptr){
        PERROR("Could not allocate write struct\n");
        goto fail;
    }

    // Initialize info structure
    png_info_ptr = png_create_info_struct(png_ptr);
    if (!png_info_ptr){
        PERROR("Could not allocate info struct\n");
        goto fail;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))){
        PERROR("Error during png creation\n");
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR (
        png_ptr,
        png_info_ptr,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    // Set title
    if (title){
        png_text title_text;
        title_text.compression  = PNG_TEXT_COMPRESSION_NONE;
        title_text.key          = "Title";
        title_text.text         = (png_charp)title;
        png_set_text(
            png_ptr,
            png_info_ptr,
            &title_text,
            1
        );
    }

    png_write_info(png_ptr, png_info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    png_row = (png_bytep) malloc(
        3 * width * sizeof (png_byte)
    );

    // Write image data
    for (y = 0; y < height; y++){
        for (x = 0; x < width; x++){
            unsigned long pixel = XGetPixel (image, x, y);

            unsigned char blue  = (pixel & blue_mask)   >> 0;
            unsigned char green = (pixel & green_mask)  >> 8;
            unsigned char red   = (pixel & red_mask)    >> 16;

            png_byte *ptr = &(png_row[x*3]);
            ptr[0] = red;
            ptr[1] = green;
            ptr[2] = blue;
        }
        png_write_row(png_ptr, png_row);
    }

    // End write
    png_write_end(png_ptr, NULL);

    // Free
    fclose(fp);

    // all ok
    err = 0;

out:
    if (png_info_ptr){
        png_free_data(
            png_ptr,
            png_info_ptr,
            PNG_FREE_ALL,
            -1
        );
    }
    if (png_ptr){
        png_destroy_write_struct(
            &png_ptr,
            (png_infopp)NULL
        );
    }
    if (png_row){
        free(png_row);
    }
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

Pixmap GetRootPixmap(
    Display *display,
    Window  *root)
{
    Pixmap          currentRootPixmap;
    Atom            act_type;
    int             act_format;
    unsigned long   nitems, bytes_after;
    unsigned char   *data = NULL;
    Atom            _XROOTPMAP_ID;

    _XROOTPMAP_ID = XInternAtom(
        display,
        "_XROOTPMAP_ID",
        False
    );

    if (XGetWindowProperty(
        display,
        *root,
        _XROOTPMAP_ID,
        0,
        1,
        False,
        XA_PIXMAP,
        &act_type,
        &act_format,
        &nitems,
        &bytes_after,
        &data) == Success)
    {
        if (data){
            currentRootPixmap = *((Pixmap *) data);
            XFree(data);
        }
    }

    return currentRootPixmap;
}

int32_t create_snapshot_to_file(
    const char *a_path)
{
    int32_t             res, err    = -1;
    //int32_t             screen      = 0;
    Display             *display    = NULL;
    XImage              *img        = NULL;
    Window              root;
    XWindowAttributes   attrs;
    //Pixmap              bg;

    display = XOpenDisplay(getenv("DISPLAY"));
    if (!display){
        PERROR("cannot connect to X server %s\n",
            getenv("DISPLAY") ? getenv("DISPLAY") : "(default)"
        );
        goto fail;
    }

    //screen = DefaultScreen(display);
    //root   = RootWindow(display, screen);

    root    = DefaultRootWindow(display);
    //display = XOpenDisplay(NULL);

    XGetWindowAttributes(display, root, &attrs);

    //bg  = GetRootPixmap(display, &root);
    img = XGetImage(
        display,
        root, //bg,
        0,
        0,
        attrs.width,
        attrs.height,
        0xffffffff,
        ZPixmap
    );
    //XFreePixmap(display, bg);

    if (!img){
        PERROR("Can't create ximage\n");
        goto fail;
    }

    if (24 != img->depth){
        PERROR("Image depth is %d. Must be 24 bits.\n",
            img->depth
        );
        goto fail;
    }

    res = write_jpeg(img, a_path);
    if (res){
        err = res;
        goto fail;
    }

    PDEBUG(100, "JPEG file successfully written to %s\n",
        a_path
    );

    // all ok
    err = 0;

out:
    if (img){
        XDestroyImage(img);
        img = NULL;
    }
    if (display){
        XCloseDisplay(display);
        display = NULL;
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

/*
int32_t create_snapshot_to_file(
    const char *a_path)
{
    int32_t             err             = 0;
    int32_t             width           = 0;
    int32_t             height          = 0;
    XImage              *image          = NULL;
    Display             *display        = NULL;
    Window              root            = 0;
    XWindowAttributes   gwa;

    display = XOpenDisplay(NULL);
    // you can get window ID via xwininfo -name XMMS
    root = DefaultRootWindow(display); // 0x480004c;

    XGetWindowAttributes(display, root, &gwa);
    width   = gwa.width;
    height  = gwa.height;

    image = XGetImage(
        display,
        root,
        0,
        0,
        width,
        height,
        AllPlanes,
        ZPixmap
    );

    //write_png(image, a_path);
    write_jpeg(image, a_path);

    if (image){
        XFree(image);
    }
    if (display){
        XCloseDisplay(display);
    }

    return err;
}
*/

int32_t create_snapshot(
    struct evbuffer *a_ev)
{
    int32_t     res, err    = -1;
    const char  *path       = "test.png";

    res = create_snapshot_to_file(path);
    if (res){
        err = res;
        PERROR("create_snapshot_to_file: '%s' filed\n",
            path
        );
        goto fail;
    }

    res = file_load(path, a_ev);
    if (0 > res){
        err = res;
        PERROR("file_load from: '%s' filed\n",
            path
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

