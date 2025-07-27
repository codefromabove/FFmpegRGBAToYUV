//
//  ConvertRGBA.c
//  FFmpegRGB
//
//  Created by Philip Schneider on 10/3/14.
//  Code was copied from ffmpeg example file:
//
//      https://www.ffmpeg.org/doxygen/2.1/decoding__encoding_8c.html
//
//  and hacked to demonstrate the conversion of RGBA data to YUV. The
//  original copyright appears below.
//

/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ConvertRGBA.h"

extern "C"
{
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

/*
 * Video encoding example
 */
static void video_encode_example(const char *filename, int codec_id)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    
    printf("Encode video file %s\n", filename);
    
    /* find the mpeg1 video encoder */
    codec = avcodec_find_encoder((enum AVCodecID)codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(2);
    }
    
    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    c->time_base = (AVRational){1,25};
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(3);
    }
    
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(4);
    }
    
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(5);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(6);
    }
    
    //
    // RGB to YUV:
    //    http://stackoverflow.com/questions/16667687/how-to-convert-rgb-from-yuv420p-for-ffmpeg-encoder
    //
    // Create some dummy RGB "frame"
    uint8_t *rgba32Data = new uint8_t[4*c->width*c->height];
    
    SwsContext * ctx = sws_getContext(c->width, c->height,
                                      AV_PIX_FMT_RGBA, c->width, c->height,
                                      AV_PIX_FMT_YUV420P, 0, 0, 0, 0);
    
    
    /* encode 1 second of video */
    for (i = 0; i < 25; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        
        
        fflush(stdout);
        /* prepare a dummy image */
        /* Y */
        //        for (y = 0; y < c->height; y++) {
        //            for (x = 0; x < c->width; x++) {
        //                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        //            }
        //        }
        //
        //        /* Cb and Cr */
        //        for (y = 0; y < c->height/2; y++) {
        //            for (x = 0; x < c->width/2; x++) {
        //                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
        //                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
        //            }
        //        }
        
        uint8_t *pos = rgba32Data;
        for (y = 0; y < c->height; y++)
        {
            for (x = 0; x < c->width; x++)
            {
                pos[0] = i / (float)25 * 255;
                pos[1] = 0;
                pos[2] = x / (float)(c->width) * 255;
                pos[3] = 255;
                pos += 4;
            }
        }

        uint8_t * inData[1] = { rgba32Data }; // RGBA32 have one plane
        //
        // NOTE: In a more general setting, the rows of your input image may
        //       be padded; that is, the bytes per row may not be 4 * width.
        //       In such cases, inLineSize should be set to that padded width.
        //
        int inLinesize[1] = { 4*c->width }; // RGBA stride
        sws_scale(ctx, inData, inLinesize, 0, c->height, frame->data, frame->linesize);
        
        frame->pts = i;
        
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(7);
        }
        
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
    
    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);
        
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(8);
        }
        
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_free_packet(&pkt);
        }
    }
    
    /* add sequence end code to have a real mpeg file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);
    
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");
}

void convertRGBA()
{
    avcodec_register_all();
    video_encode_example("/tmp/test.mpg", AV_CODEC_ID_MPEG2VIDEO);

}
