#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <libpng16/png.h>

using namespace cimg_library;

int32_t main(void)
{
    int32_t             err             = 0;
    int32_t             width           = 0;
    int32_t             height          = 0;
    XImage              *image          = NULL;
    int                 code            = 0;
    FILE                *fp             = NULL;
    Display             *display        = NULL;
    Window              root            = 0;
    const char          *title          = "Screenshot";
    unsigned long       red_mask        = 0;
    unsigned long       green_mask      = 0;
    unsigned long       blue_mask       = 0;
    png_structp         png_ptr;
    png_infop           png_info_ptr;
    png_bytep           png_row;
    XWindowAttributes   gwa;

    display = XOpenDisplay(NULL);
    root    = DefaultRootWindow(display);

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

    red_mask    = image->red_mask;
    green_mask  = image->green_mask;
    blue_mask   = image->blue_mask;

    // Open file
    fp = fopen ("test.png", "wb");
    if (fp == NULL){
        fprintf (stderr, "Could not open file for writing\n");
        code = 1;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING,
        NULL,
        NULL,
        NULL
    );
    if (png_ptr == NULL){
        fprintf (stderr, "Could not allocate write struct\n");
        code = 1;
    }

    // Initialize info structure
    png_info_ptr = png_create_info_struct (png_ptr);
    if (png_info_ptr == NULL){
        fprintf (stderr, "Could not allocate info struct\n");
        code = 1;
    }

    // Setup Exception handling
    if (setjmp (png_jmpbuf (png_ptr))){
        fprintf(stderr, "Error during png creation\n");
        code = 1;
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
    if (title != NULL){
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

    png_write_info (png_ptr, png_info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    png_row = (png_bytep) malloc (3 * width * sizeof (png_byte));

    // Write image data
    int x, y;
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
        png_write_row (png_ptr, png_row);
    }

    // End write
    png_write_end (png_ptr, NULL);

    // Free
    fclose (fp);

    if (png_info_ptr != NULL) png_free_data (png_ptr, png_info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
    if (png_row != NULL) free (png_row);

    return err;
}

