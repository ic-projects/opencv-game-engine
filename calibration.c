/**
 * @file calibration.c
 * @brief Functions to calibrate colours.
 */

/**
 * @brief A struct to hold the HSV values of the calibrated skin colour.
 */
typedef struct {
  /** The maximum Hue to match. */
  unsigned char h_max;
  /** The minimum Hue to match. */
  unsigned char h_min;
  /** The maximum Saturation to match. */
  unsigned char s_max;
  /** The minimum Saturation to match. */
  unsigned char s_min;
  /** The maximum Value to match. */
  unsigned char v_max;
  /** The minimum Value to match. */
  unsigned char v_min;
  /** True iff calibration has been done. */
  bool done;
  /** Vector of the max HSV values so they can be rendered. */
  CvScalar upper;
  /** Vector of the min HSV values so they can be rendered. */
  CvScalar lower;
} calibration_t;

/**
 * @brief Given a coordinate and a box, determines whether it is in the box.
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param box_x The x coordinate of the centre of box.
 * @param box_y The y coordinate of the centre of box.
 * @param width The distance from centre of the box to left or right edge.
 * @param height The distance from centre of the box to top or bottom edge.
 * @returns True iff the coordinate is in the box.
 */
bool in_box(int x, int y, int box_x, int box_y, int width, int height) {
  return (x > box_x - width
    && x < box_x + width
    && y > box_y - height
    && y < box_y + height);
}

/**
 * @brief Given a frame, applies a darkened border around it.
 * User will place their hand inside the undarkened centre box to calibrate
 * their skin colour.
 * @param frame The IplImage frame.
 * @param reg_x The centre box x coordinate.
 * @param reg_y The centre box y coordinate.
 * @param reg_height The centre box height.
 * @param reg_width The centre box width.
 */
void overlay_frame(IplImage *frame, int reg_x, int reg_y, int reg_height, int reg_width) {
  for (int y = 0; y < frame->height; y++) {
    for (int x = 0; x < frame->width; x++) {
      if (!in_box(x, y, reg_x, reg_y, reg_width, reg_height)) {
        for (int w = 0; w < frame->nChannels; w++) {
          frame->imageData[y * frame->widthStep + x * frame->nChannels + w] = (unsigned char) (frame->imageData[y * frame->widthStep + x * frame->nChannels + w]) / 4;
        }
      }
    }
  }
}

/**
 * @brief Calibrates according to the skin colour in centre box in the frame.
 * @param frame The IplImage frame.
 * @param c The calibration struct to place the skin colour values in.
 * @param reg_x The centre box x coordinate.
 * @param reg_y The centre box y coordinate.
 * @param reg_height The centre box height.
 * @param reg_width The centre box width.
 */
void final_calibration(IplImage *frame, calibration_t *c, int reg_x, int reg_y, int reg_height, int reg_width) {
  int size = 0;
  for (int y = reg_y - reg_height; y < reg_y + reg_height; y++) {
    for (int x = reg_x - reg_width; x < reg_x + reg_width; x++) {
      size++;
    }
  }

  uchar_array_t *h_arr = init_arr(size);
  uchar_array_t *s_arr = init_arr(size);
  uchar_array_t *v_arr = init_arr(size);
  int i = 0;

  for (int y = reg_y - reg_height; y < reg_y + reg_height; y++) {
    for (int x = reg_x - reg_width; x < reg_x + reg_width; x++) {
      h_arr->array[i] = frame->imageData[y * frame->widthStep + x * frame->nChannels];
      s_arr->array[i] = frame->imageData[y * frame->widthStep + x * frame->nChannels + 1];
      v_arr->array[i] = frame->imageData[y * frame->widthStep + x * frame->nChannels + 2];
      i++;
    }
  }

  standard_dev(h_arr);
  standard_dev(s_arr);
  standard_dev(v_arr);

  double range = 0.4;

  c->done = true;
  c->h_max = avg(h_arr);
  c->h_min = c->h_max * (1 - range);
  c->h_max *= 1 + range;
  c->s_max = avg(s_arr);
  c->s_min = c->s_max * (1 - range);
  c->s_max *= 1 + range;
  c->v_max = avg(v_arr);
  c->v_min = c->v_max * (1 - range);
  c->v_max *= 1 + range;
  c->lower = cvScalar(c->h_min, c->s_min, c->v_min);
  c->upper = cvScalar(c->h_max, c->s_max, c->v_max);

  printf("h_max: %f\n", (float) (c->h_max));
  printf("h_min: %f\n", (float) (c->h_min));
  printf("s_max: %f\n", (float) (c->s_max));
  printf("s_min: %f\n", (float) (c->s_min));
  printf("v_max: %f\n", (float) (c->v_max));
  printf("v_min: %f\n", (float) (c->v_min));
}

/**
 * @brief Calibrates according to a generic skin colour.
 * @param c The calibration struct to place the skin colour values in.
 */
void generic_calibration(calibration_t *c) {
  c->h_max = 25;
  c->h_min = 0;
  c->s_max = 150;
  c->s_min = 10;
  c->v_max = 255;
  c->v_min = 60;
  c->done = true;
}

/**
 * @brief Displays a calibration window for the user to calibrate their skin.
 * @param capture The CvCapture video stream.
 * @param calibration The calibration struct to place the skin colour values in.
 */
void calibrate(CvCapture *capture, calibration_t *calibration) {
  IplImage *frame = 0;
  calibration->done = false;

  int reg_x = 0;
  int reg_y = 0;
  int reg_height = 0;
  int reg_width = 0;
  int timer = 0;

  while ((cvWaitKey(10) != 'q' || calibration->done) && timer < 90) {
    frame = cvQueryFrame(capture);

    if (frame) {
      if (reg_x == 0) {
        reg_x = frame->width / 2;
        reg_y = frame->height / 2;
        reg_height = frame->height / 20;
        reg_width = frame->width / 20;
      }

      overlay_frame(frame,reg_x, reg_y, reg_height, reg_width);
      cvFlip(frame, frame, 1);
      cvShowImage("Calibrate", frame);
    }

    timer++;
  }
  cvCvtColor(frame, frame, CV_BGR2HSV);
  printf("%i %i %i %i\n", reg_x, reg_y, reg_height, reg_width);
  final_calibration(frame, calibration, reg_x, reg_y, reg_height, reg_width);
  cvCvtColor(frame, frame, CV_HSV2BGR);
  cvDestroyWindow("Calibrate");
}
