#include "arducam_mipicamera.h"
#include <linux/v4l2-controls.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LOG(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define SET_CONTROL 1

FILE *fd;
int frame_count = 0;
int video_callback(BUFFER *buffer) {
    if (TIME_UNKNOWN == buffer->pts) {
        // Frame data in the second half
    }
    // LOG("buffer length = %d, pts = %llu, flags = 0x%X", buffer->length, buffer->pts, buffer->flags);

    if (buffer->length) {
        if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CONFIG) {
            // SPS PPS
            if (fd) {
                fwrite(buffer->data, 1, buffer->length, fd);
                fflush(fd);
            }
        }
        if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO) {
            /// Encoder outputs inline Motion Vectors
        } else {
            // MMAL_BUFFER_HEADER_FLAG_KEYFRAME
            // MMAL_BUFFER_HEADER_FLAG_FRAME_END
            if (fd) {
                int bytes_written = fwrite(buffer->data, 1, buffer->length, fd);
                fflush(fd);
            }
            // Here may be just a part of the data, we need to check it.
            if (buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END)
                frame_count++;
        }
    }
    return 0;
}
static void default_status(VIDEO_ENCODER_STATE *state) {
    // Default everything to zero
    memset(state, 0, sizeof(VIDEO_ENCODER_STATE));
    state->encoding = VIDEO_ENCODING_H264;
    state->bitrate = 17000000;
    state->immutableInput = 1; // Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                               // the camera output or the encoder output (with compression artifacts)
    /**********************H264 only**************************************/
    state->intraperiod = -1;                  // Not set
                                              // Specify the intra refresh period (key frame rate/GoP size).
                                              // Zero to produce an initial I-frame and then just P-frames.
    state->quantisationParameter = 0;         // Quantisation parameter. Use approximately 10-40. Default 0 (off)
    state->profile = VIDEO_PROFILE_H264_HIGH; // Specify H264 profile to use for encoding
    state->level = VIDEO_LEVEL_H264_4;        // Specify H264 level to use for encoding
    state->bInlineHeaders = 0;                // Insert inline headers (SPS, PPS) to stream
    state->inlineMotionVectors = 0;           // output motion vector estimates
    state->intra_refresh_type = -1;           // Set intra refresh type
    state->addSPSTiming = 0;                  // zero or one
    state->slices = 1;
    /**********************H264 only**************************************/
}

struct reg {
    uint16_t address;
    uint16_t value;
};

struct reg regs[] = {
    {0x4F00, 0x01},
    {0x3030, 0x04},
    {0x303F, 0x01},
    {0x302C, 0x00},
    {0x302F, 0x7F},
    {0x3823, 0x00},
    {0x0100, 0x00},
    {0X3501, 0X05},
};

static const int regs_size = sizeof(regs) / sizeof(regs[0]);

int write_regs(CAMERA_INSTANCE camera_instance, struct reg regs[], int length){
    int status = 0;
    for(int i = 0; i < length; i++){
        if (arducam_write_sensor_reg(camera_instance, regs[i].address, regs[i].value)) {
            LOG("Failed to write register: 0x%02X, 0x%02X.", regs[i].address, regs[i].value);
            status += 1;
        }
    }
    return status;
}

void save_image(CAMERA_INSTANCE camera_instance, const char *name) {
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 50};
    // The actual width and height of the IMAGE_ENCODING_RAW_BAYER format and the IMAGE_ENCODING_I420 format are aligned, 
    // width 32 bytes aligned, and height 16 byte aligned.
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
    FILE *file = fopen(name, "wb");
    fwrite(buffer->data, buffer->length, 1, file);
    fclose(file);
    arducam_release_buffer(buffer);
}

int main(int argc, char **argv) {
    CAMERA_INSTANCE camera_instance;
    int width = 0, height = 0;
    LOG("Open camera...");
    int res = arducam_init_camera(&camera_instance);
    if (res) {
        LOG("init camera status = %d", res);
        return -1;
    }
    width = 1920;
    height = 1080;
    LOG("Setting the resolution...");
    res = arducam_set_resolution(camera_instance, &width, &height);
    if (res) {
        LOG("set resolution status = %d", res);
        return -1;
    } else {
        LOG("Current resolution is %dx%d", width, height);
        LOG("Notice:You can use the list_format sample program to see the resolution and control supported by the camera.");
    }
    // LOG("Start preview...");
    // PREVIEW_PARAMS preview_params = {
    //     .fullscreen = 0,             // 0 is use previewRect, non-zero to use full screen
    //     .opacity = 255,              // Opacity of window - 0 = transparent, 255 = opaque
    //     .window = {0, 0, 1280, 720}, // Destination rectangle for the preview window.
    // };
    // res = arducam_start_preview(camera_instance, &preview_params);
    // if (res) {
    //     LOG("start preview status = %d", res);
    //     return -1;
    // }
    
    // usleep(1000 * 1000 * 2);
    write_regs(camera_instance, regs, regs_size);

    // Set exposure to constant value
    arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 500);
    int exposure = 0;
    arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &exposure);
    LOG("Current exposure is %d", exposure);

    // Set white balance to constant value
    arducam_set_control(camera_instance, V4L2_CID_GAIN, 1);
    int gain = 0;
    arducam_get_control(camera_instance, V4L2_CID_GAIN, &gain);
    LOG("Current gain is %d", gain);

    // usleep(1000 * 1000 * 100);

    // LOG("Stop preview...");
    // res = arducam_stop_preview(camera_instance);
    // if (res) {
    //     LOG("stop preview status = %d", res);
    // }

        // fd = fopen("test.h264", "wb");
    fd = stdout;
    VIDEO_ENCODER_STATE video_state;
    default_status(&video_state);
    // start video callback
    // Set video_state to NULL, using default parameters
    LOG("Start video encoding...");
    res = arducam_set_video_callback(camera_instance, &video_state, video_callback, NULL);
    if (res) {
        LOG("Failed to start video encoding, probably due to resolution greater than 1920x1080 or video_state setting error.");
        return -1;
    }

    while(1)
        usleep(1000 * 300);

    // stop video callback
    LOG("Stop video encoding...");
    arducam_set_video_callback(camera_instance, NULL, NULL, NULL);
    fclose(fd);

    LOG("Close camera...");
    res = arducam_close_camera(camera_instance);
    if (res) {
        LOG("close camera status = %d", res);
    }
    return 0;
}
