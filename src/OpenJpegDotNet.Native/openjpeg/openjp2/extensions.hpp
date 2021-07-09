#ifndef _CPP_OPENJPEG_OPENJP2_EXTENSIONS_H_
#define _CPP_OPENJPEG_OPENJP2_EXTENSIONS_H_

#include "../export.hpp"
#include "../shared.hpp"

//////////////////////////////
///     bmp handling       ///
//////////////////////////////

// https://github.com/uclouvain/openjpeg/blob/v2.4.0/src/bin/jp2/convert.c
DLLEXPORT int32_t openjpeg_openjp2_extensions_imagetobmp(opj_image_t * image,
                                                         uint8_t** planes,
                                                         uint32_t* out_w,
                                                         uint32_t* out_h,
                                                         uint32_t* out_c,
                                                         uint32_t* out_p)
{
    unsigned int compno, numcomps;
    int w, h, fails;
    int line, row, curr, mask;
    int *ptr;
    unsigned char uc;
    size_t cur_buf = 0;
    uint8_t* buf = nullptr;

    if ((image->numcomps * image->x1 * image->y1) == 0)
    {
        // fprintf(stderr, "\nError: invalid raw image parameters\n");
        return 1;
    }

    numcomps = image->numcomps;

    if (numcomps > 4)
    {
        numcomps = 4;
    }

    for (compno = 1; compno < numcomps; ++compno)
    {
        if (image->comps[0].dx != image->comps[compno].dx)
        {
            break;
        }
        if (image->comps[0].dy != image->comps[compno].dy)
        {
            break;
        }
        if (image->comps[0].prec != image->comps[compno].prec)
        {
            break;
        }
        if (image->comps[0].sgnd != image->comps[compno].sgnd)
        {
            break;
        }
    }

    if (compno != numcomps)
    {
        // fprintf(stderr, "imagetoraw_common: All components shall have the same subsampling, same bit depth, same sign.\n");
        // fprintf(stderr, "\tAborting\n");
        return 1;
    }

    if (image->comps[0].prec <= 8)
    {
        *out_w = image->comps[0].w;
        *out_h = image->comps[0].h;
        *out_c = compno;
        *out_p = 8;
    }
    else if (image->comps[compno].prec <= 16)
    {
        *out_w = image->comps[0].w;
        *out_h = image->comps[0].h;
        *out_c = compno;
        *out_p = 16;
    }
    else
    {
        goto fin;
    }

    buf = (uint8_t*)calloc(*out_c, *out_w * *out_h * compno);
    *planes = buf;

    fails = 1;
    // fprintf(stdout, "Raw image characteristics: %d components\n", image->numcomps);

    for (compno = 0; compno < image->numcomps; compno++)
    {
        // fprintf(stdout, "Component %u characteristics: %dx%dx%d %s\n", compno,
        //         image->comps[compno].w,
        //         image->comps[compno].h, image->comps[compno].prec,
        //         image->comps[compno].sgnd == 1 ? "signed" : "unsigned");

        w = (int)image->comps[compno].w;
        h = (int)image->comps[compno].h;

        if (image->comps[compno].prec <= 8)
        {
            if (image->comps[compno].sgnd == 1)
            {
                mask = (1 << image->comps[compno].prec) - 1;
                ptr = image->comps[compno].data;
                for (line = 0; line < h; line++)
                {
                    for (row = 0; row < w; row++)
                    {
                        curr = *ptr;
                        if (curr > 127)
                        {
                            curr = 127;
                        }
                        else if (curr < -128)
                        {
                            curr = -128;
                        }
                        uc = (unsigned char)(curr & mask);
                        buf[cur_buf++] = uc;
                        // if (res < 1)
                        // {
                        //     fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
                        //     goto fin;
                        // }
                        ptr++;
                    }
                }
            }
            else if (image->comps[compno].sgnd == 0)
            {
                mask = (1 << image->comps[compno].prec) - 1;
                ptr = image->comps[compno].data;
                for (line = 0; line < h; line++)
                {
                    for (row = 0; row < w; row++)
                    {
                        curr = *ptr;
                        if (curr > 255)
                        {
                            curr = 255;
                        }
                        else if (curr < 0)
                        {
                            curr = 0;
                        }
                        uc = (unsigned char)(curr & mask);
                        buf[cur_buf++] = uc;
                        // if (res < 1)
                        // {
                        //     fprintf(stderr, "failed to write 1 byte for %s\n", outfile);
                        //     goto fin;
                        // }
                        ptr++;
                    }
                }
            }
        }
        else if (image->comps[compno].prec <= 16)
        {
            if (image->comps[compno].sgnd == 1)
            {
                union {
                    signed short val;
                    signed char vals[2];
                } uc16;
                mask = (1 << image->comps[compno].prec) - 1;
                ptr = image->comps[compno].data;
                for (line = 0; line < h; line++)
                {
                    for (row = 0; row < w; row++)
                    {
                        curr = *ptr;
                        if (curr > 32767)
                        {
                            curr = 32767;
                        }
                        else if (curr < -32768)
                        {
                            curr = -32768;
                        }
                        uc16.val = (signed short)(curr & mask);
                        buf[cur_buf++] = uc16.vals[0];
                        buf[cur_buf++] = uc16.vals[1];
                        // if (res < 2)
                        // {
                        //     fprintf(stderr, "failed to write 2 byte for %s\n", outfile);
                        //     goto fin;
                        // }
                        ptr++;
                    }
                }
            }
            else if (image->comps[compno].sgnd == 0)
            {
                union {
                    unsigned short val;
                    unsigned char vals[2];
                } uc16;
                mask = (1 << image->comps[compno].prec) - 1;
                ptr = image->comps[compno].data;
                for (line = 0; line < h; line++)
                {
                    for (row = 0; row < w; row++)
                    {
                        curr = *ptr;
                        if (curr > 65535)
                        {
                            curr = 65535;
                        }
                        else if (curr < 0)
                        {
                            curr = 0;
                        }
                        uc16.val = (unsigned short)(curr & mask);
                        buf[cur_buf++] = uc16.vals[0];
                        buf[cur_buf++] = uc16.vals[1];
                        // if (res < 2)
                        // {
                        //     fprintf(stderr, "failed to write 2 byte for %s\n", outfile);
                        //     goto fin;
                        // }
                        ptr++;
                    }
                }
            }
        }
        else if (image->comps[compno].prec <= 32)
        {
            // fprintf(stderr, "More than 16 bits per component not handled yet\n");
            goto fin;
        }
        else
        {
            // fprintf(stderr, "Error: invalid precision: %d\n", image->comps[compno].prec);
            goto fin;
        }
    }
    fails = 0;
fin:
    // fclose(rawFile);
    return fails;
}

//////////////////////////////
///     tga handling       ///
//////////////////////////////

/* Returns a ushort from a little-endian serialized value */
static unsigned short get_tga_ushort(const unsigned char* data)
{
    return (unsigned short)(data[0] | (data[1] << 8));
}

#define TGA_HEADER_SIZE 18

static unsigned char* tga_readheader(unsigned char *img_ptr, 
    unsigned int* bits_per_pixel, unsigned int* width, unsigned int* height, 
    int* flip_image)
{
    int palette_size;
    unsigned char tga[TGA_HEADER_SIZE];
    unsigned char id_len, /*cmap_type,*/ image_type;
    unsigned char pixel_depth, image_desc;
    unsigned short /*cmap_index,*/ cmap_len, cmap_entry_size;
    unsigned short /*x_origin, y_origin,*/ image_w, image_h;

    if (!bits_per_pixel || !width || !height || !flip_image) {
        return NULL;
    }

    //if (fread(tga, TGA_HEADER_SIZE, 1, fp) != 1) {
    //    fprintf(stderr,
    //        "\nError: fread return a number of element different from the expected.\n");
    //    return 0;
    //}
    memcpy(tga, img_ptr, TGA_HEADER_SIZE);
    img_ptr += TGA_HEADER_SIZE;

    id_len = tga[0];
    /*cmap_type = tga[1];*/
    image_type = tga[2];
    /*cmap_index = get_tga_ushort(&tga[3]);*/
    cmap_len = get_tga_ushort(&tga[5]);
    cmap_entry_size = tga[7];


#if 0
    x_origin = get_tga_ushort(&tga[8]);
    y_origin = get_tga_ushort(&tga[10]);
#endif
    image_w = get_tga_ushort(&tga[12]);
    image_h = get_tga_ushort(&tga[14]);
    pixel_depth = tga[16];
    image_desc = tga[17];

    *bits_per_pixel = (unsigned int)pixel_depth;
    *width = (unsigned int)image_w;
    *height = (unsigned int)image_h;

    /* Ignore tga identifier, if present ... */
    if (id_len) {
        //unsigned char* id = (unsigned char*)malloc(id_len);
        //if (id == 0) {
        //    fprintf(stderr, "tga_readheader: memory out\n");
        //    return 0;
        //}
        //if (!fread(id, id_len, 1, fp)) {
        //    fprintf(stderr,
        //        "\nError: fread return a number of element different from the expected.\n");
        //    free(id);
        //    return 0;
        //}
        //free(id);
        img_ptr += id_len;
    }

    /* Test for compressed formats ... not yet supported ...
    // Note :-  9 - RLE encoded palettized.
    //         10 - RLE encoded RGB. */
    if (image_type > 8) {
        //fprintf(stderr, "Sorry, compressed tga files are not currently supported.\n");
        return NULL;
    }

    *flip_image = !(image_desc & 32);

    /* Palettized formats are not yet supported, skip over the palette, if present ... */
    palette_size = cmap_len * (cmap_entry_size / 8);

    if (palette_size > 0) {
        //fprintf(stderr, "File contains a palette - not yet supported.");
        //fseek(fp, palette_size, SEEK_CUR);
        img_ptr += palette_size;
    }
    return img_ptr;
}

#ifdef OPJ_BIG_ENDIAN

static INLINE OPJ_UINT16 swap16(OPJ_UINT16 x)
{
    return (OPJ_UINT16)(((x & 0x00ffU) << 8) | ((x & 0xff00U) >> 8));
}

#endif

static unsigned char* tga_writeheader(unsigned char* img_ptr, int bits_per_pixel,
    int width, int height, OPJ_BOOL flip_image)
{
    OPJ_UINT16 image_w, image_h, us0;
    unsigned char uc0, image_type;
    unsigned char pixel_depth, image_desc;

    if (!bits_per_pixel || !width || !height) {
        return 0;
    }

    pixel_depth = 0;

    if (bits_per_pixel < 256) {
        pixel_depth = (unsigned char)bits_per_pixel;
    }
    else {
        fprintf(stderr, "ERROR: Wrong bits per pixel inside tga_header");
        return 0;
    }
    uc0 = 0;

    //if (fwrite(&uc0, 1, 1, fp) != 1) {
    //    goto fails;    /* id_length */
    //}
    memcpy(img_ptr, &uc0, 1);
    img_ptr++;

    //if (fwrite(&uc0, 1, 1, fp) != 1) {
    //    goto fails;    /* colour_map_type */
    //}
    memcpy(img_ptr, &uc0, 1);
    img_ptr++;

    image_type = 2; /* Uncompressed. */
    //if (fwrite(&image_type, 1, 1, fp) != 1) {
    //    goto fails;
    //}
    memcpy(img_ptr, &image_type, 1);
    img_ptr++;

    us0 = 0;
    //if (fwrite(&us0, 2, 1, fp) != 1) {
    //    goto fails;    /* colour_map_index */
    //}
    memcpy(img_ptr, &us0, 2);
    img_ptr += 2;

    //if (fwrite(&us0, 2, 1, fp) != 1) {
    //    goto fails;    /* colour_map_length */
    //}
    memcpy(img_ptr, &us0, 2);
    img_ptr += 2;

    //if (fwrite(&uc0, 1, 1, fp) != 1) {
    //    goto fails;    /* colour_map_entry_size */
    //}
    memcpy(img_ptr, &uc0, 1);
    img_ptr++;

    //if (fwrite(&us0, 2, 1, fp) != 1) {
    //    goto fails;    /* x_origin */
    //}
    memcpy(img_ptr, &us0, 2);
    img_ptr += 2;

    //if (fwrite(&us0, 2, 1, fp) != 1) {
    //    goto fails;    /* y_origin */
    //}
    memcpy(img_ptr, &us0, 2);
    img_ptr += 2;

    image_w = (unsigned short)width;
    image_h = (unsigned short)height;

#ifndef OPJ_BIG_ENDIAN
    //if (fwrite(&image_w, 2, 1, fp) != 1) {
    //    goto fails;
    //}
    //if (fwrite(&image_h, 2, 1, fp) != 1) {
    //    goto fails;
    //}
    memcpy(img_ptr, &image_w, 2);
    img_ptr += 2;
    memcpy(img_ptr, &image_h, 2);
    img_ptr += 2;
#else
    image_w = swap16(image_w);
    image_h = swap16(image_h);
    //if (fwrite(&image_w, 2, 1, fp) != 1) {
    //    goto fails;
    //}
    //if (fwrite(&image_h, 2, 1, fp) != 1) {
    //    goto fails;
    //}
    memcpy(img_ptr, &image_w, 2);
    img_ptr += 2;
    memcpy(img_ptr, &image_h, 2);
    img_ptr += 2;
#endif

    //if (fwrite(&pixel_depth, 1, 1, fp) != 1) {
    //    goto fails;
    //}
    memcpy(img_ptr, &pixel_depth, 1);
    img_ptr++;

    image_desc = 8; /* 8 bits per component. */

    if (flip_image) {
        image_desc |= 32;
    }
    //if (fwrite(&image_desc, 1, 1, fp) != 1) {
    //    goto fails;
    //}
    memcpy(img_ptr, &image_desc, 1);
    img_ptr++;

    return img_ptr;

//fails:
//    fputs("\nwrite_tgaheader: write ERROR\n", stderr);
//    return NULL;
}

DLLEXPORT int32_t openjpeg_openjp2_extensions_imagetotga(opj_image_t * image, 
                                                         uint8_t** out_tga, 
                                                         uint32_t* out_size,
                                                         uint32_t* out_w,
                                                         uint32_t* out_h,
                                                         uint32_t* out_c)
{
    uint8_t *tga, *tga_ptr;
    int width, height, bpp, x, y;
    OPJ_BOOL write_alpha;
    unsigned int i;
    int adjustR, adjustG = 0, adjustB = 0, fails;
    unsigned int alpha_channel;
    float r, g, b, a;
    unsigned char value;
    float scale;
    fails = 1;

    for (i = 0; i < image->numcomps - 1; i++) {
        if ((image->comps[0].dx != image->comps[i + 1].dx)
            || (image->comps[0].dy != image->comps[i + 1].dy)
            || (image->comps[0].prec != image->comps[i + 1].prec)
            || (image->comps[0].sgnd != image->comps[i + 1].sgnd)) {
            //fprintf(stderr,
            //    "Unable to create a tga file with such J2K image charateristics.\n");
            return 1;
        }
    }

    *out_w = width = (int)image->comps[0].w;
    *out_h = height = (int)image->comps[0].h;
    *out_c = (int)image->numcomps;

    /* Mono with alpha, or RGB with alpha. */
    write_alpha = (image->numcomps == 2) || (image->numcomps == 4);

    /* Write TGA header  */
    bpp = write_alpha ? 32 : 24;

    *out_size = TGA_HEADER_SIZE + image->numcomps * width * height;
    tga = (uint8_t*)malloc(*out_size);
    if (tga == NULL) { goto fin; }
    tga_ptr = tga_writeheader(tga, bpp, width, height, OPJ_TRUE);
    if (tga_ptr == NULL) {
        free(tga);
        goto fin;
    }

    alpha_channel = image->numcomps - 1;

    scale = 255.0f / (float)((1 << image->comps[0].prec) - 1);

    adjustR = (image->comps[0].sgnd ? 1 << (image->comps[0].prec - 1) : 0);
    if (image->numcomps >= 3) {
        adjustG = (image->comps[1].sgnd ? 1 << (image->comps[1].prec - 1) : 0);
        adjustB = (image->comps[2].sgnd ? 1 << (image->comps[2].prec - 1) : 0);
    }

    for (y = 0; y < height; y++) {
        unsigned int index = (unsigned int)(y * width);

        for (x = 0; x < width; x++, index++) {
            r = (float)(image->comps[0].data[index] + adjustR);

            if (image->numcomps > 2) {
                g = (float)(image->comps[1].data[index] + adjustG);
                b = (float)(image->comps[2].data[index] + adjustB);
            }
            else {
                /* Greyscale ... */
                g = r;
                b = r;
            }

            /* TGA format writes BGR ... */
            if (b > 255.) {
                b = 255.;
            }
            else if (b < 0.) {
                b = 0.;
            }
            value = (unsigned char)(b * scale);
            memset(tga_ptr, value, 1);
            tga_ptr++;

            if (g > 255.) {
                g = 255.;
            }
            else if (g < 0.) {
                g = 0.;
            }
            value = (unsigned char)(g * scale);
            memset(tga_ptr, value, 1);
            tga_ptr++;

            if (r > 255.) {
                r = 255.;
            }
            else if (r < 0.) {
                r = 0.;
            }
            value = (unsigned char)(r * scale);
            memset(tga_ptr, value, 1);
            tga_ptr++;

            if (write_alpha) {
                a = (float)(image->comps[alpha_channel].data[index]);
                if (a > 255.) {
                    a = 255.;
                }
                else if (a < 0.) {
                    a = 0.;
                }
                value = (unsigned char)(a * scale);
                memset(tga_ptr, value, 1);
                tga_ptr++;
            }
        }
    }
    fails = 0;
    *out_tga = tga;
fin:
    return fails;
}

DLLEXPORT int32_t openjpeg_openjp2_extensions_tgatoimage(uint8_t * tga_image,
                                                         opj_image_t ** out_image,
                                                         opj_cparameters_t * parameters)
{
    opj_image_t* image;
    unsigned int image_width, image_height, pixel_bit_depth;
    unsigned int x, y;
    int flip_image = 0;
    opj_image_cmptparm_t cmptparm[4];   /* maximum 4 components */
    int numcomps;
    OPJ_COLOR_SPACE color_space;
    OPJ_BOOL mono;
    OPJ_BOOL save_alpha;
    int fails = 1;

    uint8_t* tga_ptr = tga_readheader(tga_image, &pixel_bit_depth, &image_width, 
        &image_height, &flip_image);
    if (tga_ptr == NULL) {
        return fails;
    }

    /* We currently only support 24 & 32 bit tga's ... */
    if (!((pixel_bit_depth == 24) || (pixel_bit_depth == 32))) {
        return fails;
    }

    /* initialize image components */
    memset(&cmptparm[0], 0, 4 * sizeof(opj_image_cmptparm_t));

    mono = (pixel_bit_depth == 8) ||
        (pixel_bit_depth == 16);  /* Mono with & without alpha. */
    save_alpha = (pixel_bit_depth == 16) ||
        (pixel_bit_depth == 32); /* Mono with alpha, or RGB with alpha */

    if (mono) {
        color_space = OPJ_CLRSPC_GRAY;
        numcomps = save_alpha ? 2 : 1;
    }
    else {
        numcomps = save_alpha ? 4 : 3;
        color_space = OPJ_CLRSPC_SRGB;
    }

    for (int i = 0; i < numcomps; i++) {
        cmptparm[i].prec = 8;
        cmptparm[i].bpp = 8;
        cmptparm[i].sgnd = 0;
        cmptparm[i].dx = (OPJ_UINT32)parameters->subsampling_dx;
        cmptparm[i].dy = (OPJ_UINT32)parameters->subsampling_dy;
        cmptparm[i].w = image_width;
        cmptparm[i].h = image_height;
    }

    /* create the image */
    image = opj_image_create((OPJ_UINT32)numcomps, &cmptparm[0], color_space);

    if (!image) { return fails; }

    /* set image offset and reference grid */
    image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
    image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
    image->x1 = !image->x0 ? (OPJ_UINT32)(image_width - 1) *
        (OPJ_UINT32)parameters->subsampling_dx + 1 : image->x0 + (OPJ_UINT32)(image_width - 1) *
        (OPJ_UINT32)parameters->subsampling_dx + 1;
    image->y1 = !image->y0 ? (OPJ_UINT32)(image_height - 1) *
        (OPJ_UINT32)parameters->subsampling_dy + 1 : image->y0 + (OPJ_UINT32)(image_height - 1) *
        (OPJ_UINT32)parameters->subsampling_dy + 1;

    /* set image data */
    for (y = 0; y < image_height; y++) {
        int index;

        if (flip_image) {
            index = (int)((image_height - y - 1) * image_width);
        }
        else {
            index = (int)(y * image_width);
        }

        if (numcomps == 3) {
            for (x = 0; x < image_width; x++) {
                unsigned char r, g, b;

                memset(&b, *tga_ptr, 1);
                tga_ptr++;

                memset(&g, *tga_ptr, 1);
                tga_ptr++;

                memset(&r, *tga_ptr, 1);

                image->comps[0].data[index] = r;
                image->comps[1].data[index] = g;
                image->comps[2].data[index] = b;
                index++;
            }
        }
        else if (numcomps == 4) {
            for (x = 0; x < image_width; x++) {
                unsigned char r, g, b, a;

                memset(&b, *tga_ptr, 1);
                tga_ptr++;

                memset(&g, *tga_ptr, 1);
                tga_ptr++;

                memset(&r, *tga_ptr, 1);
                tga_ptr++;

                memset(&a, *tga_ptr, 1);
                tga_ptr++;

                image->comps[0].data[index] = r;
                image->comps[1].data[index] = g;
                image->comps[2].data[index] = b;
                image->comps[3].data[index] = a;
                index++;
            }
        } else {
        //    fprintf(stderr, "Currently unsupported bit depth : %s\n", filename);
            return fails;
        }
    }
    *out_image = image;
    return 0;
}

#endif // _CPP_OPENJPEG_OPENJP2_EXTENSIONS_H_
