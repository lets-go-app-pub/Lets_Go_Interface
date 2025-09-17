//
// Created by jeremiah on 9/18/21.
//

#pragma once

//picture size on android is set to 1930 x 1200
const int JPEG_HEIGHT = 1930;
const int JPEG_WIDTH = 1200;
const int JPEG_THUMBNAIL_CROPPED_HEIGHT = 200; //this is the height in pixels the thumbnail will be saved as
const int JPEG_THUMBNAIL_CROPPED_WIDTH = JPEG_THUMBNAIL_CROPPED_HEIGHT * JPEG_WIDTH / JPEG_HEIGHT; //this is the width in pixels the thumbnail will be saved as
const int JPEG_IMAGE_QUALITY_VALUE = 50;


