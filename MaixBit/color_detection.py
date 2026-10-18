import sensor, image, time, ustruct
from machine import UART
from fpioa_manager import fm


# UART1 sends recognition events to the car controller.
fm.register(24, fm.fpioa.UART1_TX, force=True)
fm.register(25, fm.fpioa.UART1_RX, force=True)
uart_car = UART(UART.UART1, 115200, 8, 0, 1,
                timeout=1000, read_buf_len=4096)


# Fixed camera parameters reduce color drift between frames.
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.set_vflip(True)
sensor.set_hmirror(False)

# Fixed camera parameters for indoor / low-light color tracking.
# Keep auto gain and white balance disabled so the LAB color does not drift.
sensor.set_auto_exposure(False, exposure_us=35000)
sensor.set_auto_gain(False, 18)
sensor.set_auto_whitebal(False)
sensor.set_brightness(2)
sensor.set_contrast(1)
sensor.set_saturation(1)

# Let the sensor settle after all camera parameters have been applied.
sensor.skip_frames(time=2000)

clock = time.clock()


# LAB threshold for blue. Tune it under the real lighting conditions.
BLUE_THRESHOLD = (5, 85, -30, 40, -80, -5)

# Use most of the 320 x 240 image while ignoring only the outermost border.
DETECT_ROI = (10, 10, 300, 220)

BLUE_PIXELS_THRESHOLD = 120
BLUE_AREA_THRESHOLD = 200
BLUE_MIN_FRAMES = 2
BLUE_RELEASE_FRAMES = 4

# Red stop detection is intentionally stricter than blue counting.
# It only watches the small central area where a stop marker should appear.
RED_THRESHOLD = (15, 75, 35, 80, 20, 70)
RED_STOP_ROI = (130, 90, 60, 60)
RED_PIXELS_THRESHOLD = 500
RED_AREA_THRESHOLD = 800
RED_MIN_FRAMES = 3
RED_RELEASE_FRAMES = 8

# UART frame: 0xAA, command, value, 0x55.
CMD_BLUE_COUNT = 0x01
CMD_RED_STOP = 0x02

blue_count = 0
blue_seen_frames = 0
blue_lost_frames = 0
blue_active = False
red_seen_frames = 0
red_lost_frames = 0
red_active = False


def send_blue_count(count):
    frame = ustruct.pack("<BBBB", 0xAA, CMD_BLUE_COUNT,
                         count & 0xFF, 0x55)
    # Send the absolute count three times. Duplicate frames are harmless,
    # and repetition makes a one-way UART link more reliable.
    for i in range(3):
        uart_car.write(frame)
        time.sleep_ms(5)
    print("BLUE count:", count)


def send_red_stop():
    frame = ustruct.pack("<BBBB", 0xAA, CMD_RED_STOP, 0x01, 0x55)
    # A stop command is safety-critical, so send it several times.
    for i in range(5):
        uart_car.write(frame)
        time.sleep_ms(5)
    print("RED stop")


def find_largest_valid_blue(img):
    blobs = img.find_blobs([BLUE_THRESHOLD],
                           roi=DETECT_ROI,
                           pixels_threshold=BLUE_PIXELS_THRESHOLD,
                           area_threshold=BLUE_AREA_THRESHOLD,
                           merge=True,
                           margin=10)

    largest = None
    largest_pixels = 0

    for blob in blobs:
        if blob.w() < 8 or blob.h() < 8:
            continue

        if blob.pixels() > largest_pixels:
            largest = blob
            largest_pixels = blob.pixels()

    return largest


def find_largest_valid_red(img):
    blobs = img.find_blobs([RED_THRESHOLD],
                           roi=RED_STOP_ROI,
                           pixels_threshold=RED_PIXELS_THRESHOLD,
                           area_threshold=RED_AREA_THRESHOLD,
                           merge=True,
                           margin=5)

    largest = None
    largest_pixels = 0

    for blob in blobs:
        if blob.w() < 20 or blob.h() < 20:
            continue

        if blob.pixels() > largest_pixels:
            largest = blob
            largest_pixels = blob.pixels()

    return largest


while True:
    clock.tick()
    img = sensor.snapshot()
    img = img.mean(1)

    img.draw_rectangle(DETECT_ROI, color=(255, 255, 255),
                       thickness=1, fill=False)
    img.draw_rectangle(RED_STOP_ROI, color=(255, 0, 0),
                       thickness=2, fill=False)

    red_blob = find_largest_valid_red(img)
    red_detected = red_blob is not None

    if red_detected:
        red_seen_frames += 1
        red_lost_frames = 0

        img.draw_rectangle(red_blob.rect(), color=(255, 0, 0),
                           thickness=3, fill=False)
        img.draw_cross(red_blob.cx(), red_blob.cy(),
                       color=(255, 0, 0), size=12, thickness=2)
        img.draw_string(red_blob.x(), max(0, red_blob.y() - 18),
                        "RED STOP", color=(255, 0, 0), scale=2)

        if red_seen_frames >= RED_MIN_FRAMES and not red_active:
            red_active = True
            send_red_stop()
    else:
        red_seen_frames = 0
        red_lost_frames += 1

        if red_lost_frames >= RED_RELEASE_FRAMES:
            red_active = False

    blue_blob = find_largest_valid_blue(img)
    blue_detected = blue_blob is not None

    if blue_detected:
        blue_seen_frames += 1
        blue_lost_frames = 0

        img.draw_rectangle(blue_blob.rect(), color=(0, 0, 255),
                           thickness=3, fill=False)
        img.draw_cross(blue_blob.cx(), blue_blob.cy(),
                       color=(0, 0, 255), size=10, thickness=2)
        img.draw_string(blue_blob.x(), max(0, blue_blob.y() - 18),
                        "BLUE", color=(0, 0, 255), scale=2)

        # Count only when a new object becomes stable.
        if blue_seen_frames >= BLUE_MIN_FRAMES and not blue_active:
            blue_active = True
            blue_count += 1
            send_blue_count(blue_count)
    else:
        blue_seen_frames = 0
        blue_lost_frames += 1

        # Allow another count only after the current object disappears.
        if blue_lost_frames >= BLUE_RELEASE_FRAMES:
            blue_active = False

    img.draw_string(5, 5, "BLUE:{}".format(blue_count),
                    color=(0, 0, 255), scale=2)
    img.draw_string(5, 27, "FPS:{:.1f}".format(clock.fps()),
                    color=(255, 255, 255), scale=1)

