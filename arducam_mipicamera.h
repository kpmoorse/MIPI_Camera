#ifndef _ARDUCAM_MIPI_CAMERA_H__
#define _ARDUCAM_MIPI_CAMERA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define FOURCC(a, b, c, d) ((a) | (b << 8) | (c << 16) | (d << 24))

#define IMAGE_ENCODING_I420 FOURCC('I', '4', '2', '0')
#define IMAGE_ENCODING_JPEG FOURCC('J', 'P', 'E', 'G')
#define IMAGE_ENCODING_RAW_BAYER FOURCC('R', 'A', 'W', ' ')

#define VIDEO_ENCODING_H264 FOURCC('H', '2', '6', '4')

#define OUTPUT_FLAG_KEEP_BUFFER_REQUIREMENTS 0x08
#define OUTPUT_FLAG_BUFFER_ALLOCATION_USE_MMAL_CORE 0x10

#define VIDEO_LEVEL_H264_4 0x1C
#define VIDEO_LEVEL_H264_41 0x1D
#define VIDEO_LEVEL_H264_42 0x1E

#define VIDEO_PROFILE_H264_BASELINE 0x19
#define VIDEO_PROFILE_H264_MAIN 0x1A
#define VIDEO_PROFILE_H264_HIGH 0x1C

#define VIDEO_INTRA_REFRESH_CYCLIC 0x00
#define VIDEO_INTRA_REFRESH_ADAPTIVE 0x01
#define VIDEO_INTRA_REFRESH_BOTH 0x02
#define VIDEO_INTRA_REFRESH_CYCLIC_MROWS 0x7F000001
/** \name Special Unknown Time Value
 * Timestamps in MMAL are defined as signed 64 bits integer values representing microseconds.
 * However a pre-defined special value is used to signal that a timestamp is not known. */
/* @{ */
#define TIME_UNKNOWN (INT64_C(1) << 63) /**< Special value signalling that time is not known */
/* @} */

/** \name Buffer header flags
 * \anchor bufferheaderflags
 * The following flags describe properties of a buffer header */
/* @{ */
/** Signals that the current payload is the end of the stream of data */
#define MMAL_BUFFER_HEADER_FLAG_EOS (1 << 0)
/** Signals that the start of the current payload starts a frame */
#define MMAL_BUFFER_HEADER_FLAG_FRAME_START (1 << 1)
/** Signals that the end of the current payload ends a frame */
#define MMAL_BUFFER_HEADER_FLAG_FRAME_END (1 << 2)
/** Signals that the current payload contains only complete frames (1 or more) */
#define MMAL_BUFFER_HEADER_FLAG_FRAME (MMAL_BUFFER_HEADER_FLAG_FRAME_START | MMAL_BUFFER_HEADER_FLAG_FRAME_END)
/** Signals that the current payload is a keyframe (i.e. self decodable) */
#define MMAL_BUFFER_HEADER_FLAG_KEYFRAME (1 << 3)
/** Signals a discontinuity in the stream of data (e.g. after a seek).
 * Can be used for instance by a decoder to reset its state */
#define MMAL_BUFFER_HEADER_FLAG_DISCONTINUITY (1 << 4)
/** Signals a buffer containing some kind of config data for the component
 * (e.g. codec config data) */
#define MMAL_BUFFER_HEADER_FLAG_CONFIG (1 << 5)
/** Signals an encrypted payload */
#define MMAL_BUFFER_HEADER_FLAG_ENCRYPTED (1 << 6)
/** Signals a buffer containing side information */
#define MMAL_BUFFER_HEADER_FLAG_CODECSIDEINFO (1 << 7)
/** Signals a buffer which is the snapshot/postview image from a stills capture */
#define MMAL_BUFFER_HEADER_FLAGS_SNAPSHOT (1 << 8)
/** Signals a buffer which contains data known to be corrupted */
#define MMAL_BUFFER_HEADER_FLAG_CORRUPTED (1 << 9)
/** Signals that a buffer failed to be transmitted */
#define MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED (1 << 10)
/** Signals the output buffer won't be used, just update reference frames */
#define MMAL_BUFFER_HEADER_FLAG_DECODEONLY (1 << 11)
/** Signals that the end of the current payload ends a NAL */
#define MMAL_BUFFER_HEADER_FLAG_NAL_END (1 << 12)
/* @} */

typedef struct {
    uint32_t encoding;
    int quality; // JPEG quality setting (1-100)
} IMAGE_FORMAT;

/** Describes a rectangle */
typedef struct {
    int32_t x;      /**< x coordinate (from left) */
    int32_t y;      /**< y coordinate (from top) */
    int32_t width;  /**< width */
    int32_t height; /**< height */
} RECTANGLE;

typedef struct {
    int fullscreen;   // 0 is use previewRect, non-zero to use full screen
    int opacity;      // Opacity of window - 0 = transparent, 255 = opaque
    RECTANGLE window; // Destination rectangle for the preview window.
} PREVIEW_PARAMS;

typedef struct
{
    uint32_t encoding;         /// Requested codec video encoding (MJPEG or H264)
    int bitrate;               /// Requested bitrate
    int intraperiod;           /// Intra-refresh period (key frame rate)
    int quantisationParameter; /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
    int bInlineHeaders;        /// Insert inline headers to stream (SPS, PPS)
    int immutableInput;        /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
                               /// the camera output or the encoder output (with compression artifacts)
    int profile;               /// H264 profile to use for encoding
    int level;                 /// H264 level to use for encoding

    int inlineMotionVectors; /// Encoder outputs inline Motion Vectors
    int intra_refresh_type;  /// What intra refresh type to use. -1 to not set.
    int addSPSTiming;        /// 0 or 1
    int slices;              /// Horizontal slices per frame. Default 1 (off)
} VIDEO_ENCODER_STATE;

typedef struct {
    void *priv; /**< This is private data, please don't change it. */
    uint8_t *data;
    uint32_t alloc_size; /**< Allocated size in bytes of payload buffer */
    uint32_t length;     /**< Number of bytes currently used in the payload buffer (starting
                                   from offset) */
    uint32_t flags;      /**< Flags describing properties of a buffer header (see
                                   \ref bufferheaderflags "Buffer header flags") */

    int64_t pts;    /**< Presentation timestamp in microseconds. \ref TIME_UNKNOWN
                                   is used when the pts is unknown. */
    void *userdata; /**< Field reserved for use by the client */
} BUFFER;

struct format {
    int width;
    int height;
};
struct camera_ctrl {
    int id;
    const char *desc;
    int max_value;
    int min_value;
    int default_value;
};

struct camera_interface {
    int i2c_bus;        // /dev/i2c-0  or /dev/i2c-1
    int camera_num;     // mipi interface num
    int sda_pins[2];    // enable sda_pins[camera_num], disable sda_pins[camera_num ? 0 : 1]
    int scl_pins[2];    // enable scl_pins[camera_num], disable scl_pins[camera_num ? 0 : 1]
    int shutdown_pins[2];
    int led_pins[2];
};

// note The buffer will be automatically released after the callback function ends.
typedef int (*OUTPUT_CALLBACK)(BUFFER *buffer);

typedef void *CAMERA_INSTANCE;

/**
 * init camera.
 * @param camera_instance Pointer of type CAMERA_INSTANCE, use to obtain CAMERA_INSTANCE.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    CAMERA_INStANCE camera_instance;
    arducam_init_camera(&camera_instance);
 @endcode
 * */
int arducam_init_camera(CAMERA_INSTANCE *camera_instance);

/**
 * init camera.
 * @param camera_instance Pointer of type CAMERA_INSTANCE, use to obtain CAMERA_INSTANCE.
 * @param camera_num Camera interface num.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    CAMERA_INStANCE camera_instance;
    arducam_init_camera(&camera_instance, 0);
 @endcode
 @note Some boards have multiple camera interfaces.
 * */
int arducam_init_camera2(CAMERA_INSTANCE *camera_instance, struct camera_interface cam_interface);


/**
 * Set output resolution.
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param width Pointer of type int, Used to specify the width and return to the actual width.
 * @param height Pointer of type int, Used to specify the height and return to the actual height.
 * @return error code , 0 success, !0 error.
 * */
int arducam_set_resolution(CAMERA_INSTANCE camera_instance, int *width, int *height);

/**
 * Set video data output callback.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param encoder_state Used to specify encoding parameters. Use default parameters if NULL.
 * @param callback Callback method, this method will be called when there is data return.
 * @param userdata Userdata, which will be a member of the buffer parameter in the callback function.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    int video_callback(BUFFER *buffer) {
        buffer->userdata;
    }
    // start callback
    if(arducam_set_video_callback(camera_instance, NULL, video_callback, NULL)){
        printf("set video callback failed.");
    }
    // stop callback
    arducam_set_video_callback(camera_instance, NULL, NULL, NULL)
 @endcode
 @note Calling the arducam_set_resolution function before stopping video encoding will terminate the video encoding.
 @note The buffer will be automatically released after the callback function ends.
 * */
int arducam_set_video_callback(CAMERA_INSTANCE camera_instance, VIDEO_ENCODER_STATE *encoder_state, OUTPUT_CALLBACK callback, void *userdata);

/**
 * Set raw data output callback.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param callback Callback method, this method will be called when there is data return.
 * @param userdata Userdata, which will be a member of the buffer parameter in the callback function.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    int raw_callback(BUFFER *buffer) {
        buffer->userdata;
    }
    // start callback
    if(arducam_set_raw_callback(camera_instance, raw_callback, NULL)){
        printf("set raw data callback failed.");
    }
    // stop callback
    arducam_set_raw_callback(camera_instance, NULL, NULL)
 @endcode
 @note If you do a time-consuming operation in the callback, it will affect other parts, such as preview, video encoding(This issue may be fixed in the future).
 @note The buffer will be automatically released after the callback function ends.
 * */
int arducam_set_raw_callback(CAMERA_INSTANCE camera_instance, OUTPUT_CALLBACK callback, void *userdata);

/**
 * Get single frame data.
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param format The data format to be obtained.
 * @param timeout This method will return NULL if no data is obtained at this time.
 * @return BUFFER structure pointer containing image data.
 * 
 * example:
 @code
    IMAGE_FORMAT fmt = {IMAGE_ENCODING_JPEG, 50};
    BUFFER *buffer = arducam_capture(camera_instance, &fmt, 3000);
    if (!buffer) {
        LOG("capture timeout.");
        return;
    }
 @endcode
 @note Currently supported image formats are:
    IMAGE_ENCODING_JPEG, IMAGE_ENCODING_I420, IMAGE_ENCODING_RAW_BAYER
 @note When the buffer is used, you need to call the arducam_release_buffer function to release it.
 @note The actual width and height of the raw bayer format and the yuv420 format are aligned, width 32 bytes aligned, and height 16 byte aligned.
 * */
BUFFER *arducam_capture(CAMERA_INSTANCE camera_instance, IMAGE_FORMAT *format, int timeout);

/**
 * Used to release the memory occupied by the buffer.
 * 
 * @param buffer The buffer to be released.
 * */
void arducam_release_buffer(BUFFER *buffer);

/**
 * Turn on image preview
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param preview_params Preview parameter,Use default parameters if NULL.
 * @return error code , 0 success, !0 error.
 * */
int arducam_start_preview(CAMERA_INSTANCE camera_instance, PREVIEW_PARAMS *preview_params);

/**
 * Turn off image preview
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @return error code , 0 success, !0 error.
 * */
int arducam_stop_preview(CAMERA_INSTANCE camera_instance);

/**
 * Release all resources and turn off the camera.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @return error code , 0 success, !0 error.
 * */
int arducam_close_camera(CAMERA_INSTANCE camera_instance);

/**
 * Set camera control to default value.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param ctrl_id Control id.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    arducam_reset_control(camera_instance, V4L2_CID_EXPOSURE);
 @endcode
 * */
int arducam_reset_control(CAMERA_INSTANCE *camera_instance, int ctrl_id);

/**
 * Set camera control.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param ctrl_id Control id.
 * @param value Control value.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    arducam_set_control(camera_instance, V4L2_CID_EXPOSURE, 3000);
 @endcode
 * */
int arducam_set_control(CAMERA_INSTANCE camera_instance, int ctrl_id, int value);

/**
 * Set camera control value.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param ctrl_id Control id.
 * @param value Current control value.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    int exposure = 0;
    arducam_get_control(camera_instance, V4L2_CID_EXPOSURE, &exposure);
    printf("Current exposure is %d\n", exposure);
 @endcode
 * */
int arducam_get_control(CAMERA_INSTANCE camera_instance, int ctrl_id, int *value);

/**
 * Get the resolution supported by the current camera
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param fmt Used to return resolution parameters.
 * @param index Format list index.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    struct format support_fmt;
    unsigned int index = 0;
    while (!arducam_get_support_formats(camera_instance, &support_fmt, index++)) {
        printf("index: %d, width: %d, height: %d\n", index, support_fmt.width, support_fmt.height);
    }
 @endcode
 * */
int arducam_get_support_formats(CAMERA_INSTANCE camera_instance, struct format *fmt, int index);

/**
 * Get the control parameters supported by the current camera.
 * 
 * @param camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param cam_ctrl Used to return control parameters.
 * @param index Control list index.
 * @return error code , 0 success, !0 error.
 * 
 * example:
 @code
    unsigned int index = 0;
    struct camera_ctrl support_cam_ctrl;
    while (!arducam_get_support_controls(camera_instance, &support_cam_ctrl, index++)) {
        int value = 0;
        if (arducam_get_control(camera_instance, support_cam_ctrl.id, &value)) {
            printf("Get ctrl %s fail.\n", support_cam_ctrl.desc);
        }
        printf("index: %d, CID: 0x%08X, desc: %s, min: %d, max: %d, default: %d, current: %d\n",
            index, support_cam_ctrl.id, support_cam_ctrl.desc, support_cam_ctrl.min_value,
            support_cam_ctrl.max_value, support_cam_ctrl.default_value, value);
    }
 @endcode
 * */
int arducam_get_support_controls(CAMERA_INSTANCE camera_instance, struct camera_ctrl *cam_ctrl, int index);

/**
 * @brief Write sensor register.
 * 
 * @param camera_instance camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param address Sensor register address.
 * @param value The value you want to write
 * @return error code , 0 success, !0 error.
 */
int arducam_write_sensor_reg(CAMERA_INSTANCE camera_instance, uint16_t address, uint16_t value);

/**
 * @brief Read sensor register.
 * 
 * @param camera_instance camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param address Sensor register address.
 * @param value The address of the variable that stores the result.
 * @return error code , 0 success, !0 error.
 */
int arducam_read_sensor_reg(CAMERA_INSTANCE camera_instance, uint16_t address, uint16_t *value);

/**
 * @brief Enable/Disable software auto exposure.
 * 
 * @param camera_instance camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param enable 0 disable, !0 enable.
 * @return error code , 0 success, !0 error.
 * 
 * @note Calling the arducam_set_resolution function will turn off this feature.
 */
int arducam_software_auto_exposure(CAMERA_INSTANCE camera_instance, int enable);

/**
 * @brief Enable/Disable software auto white balance.
 * 
 * @param camera_instance camera_instance Type CAMERA_INSTANCE, Obtained from arducam_init_camera function.
 * @param enable 0 disable, !0 enable.
 * @return error code , 0 success, !0 error.
 * 
 * @note Calling the arducam_set_resolution function will turn off this feature.
 */
int arducam_software_auto_white_balance(CAMERA_INSTANCE camera_instance, int enable);

#ifdef __cplusplus
}
#endif

#endif