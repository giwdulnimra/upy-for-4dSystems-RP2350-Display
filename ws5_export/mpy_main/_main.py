# main.py
import graphics4d as gfx
consts = {"PROJECT_ORIENTATION": "PORTRAIT_R",
          "iForm0": 0,
          "iSliderA0": 1,
          "iSliderA0Fill": 2,
          "iSliderA0Thumb": 3,
          "iGaugeB0": 4,
          "SLIDER_A0_INFO": [50],
          }
#import consts
#import config
#display, touch = config.start()

# create instance
display = gfx.init()

display.Screenmode(consts.PROJECT_ORIENTATION)
display.Contrast(8)
# initialize touch
touch = gfx.Touch4D()

# Load embedded gcx
hndl = display.LoadImageControl(b'' )  # if you implemented buffer overload and want to pass bytes
# OR, using explicit helper:
hndl_index = display.load_embedded_graphics()

# if your wrapper returns an index, use it:
img_index = hndl_index

# Set properties using list from consts
# (some wrappers accept list/tuple directly, others expect bytes; adapt accordingly)
display.SetProperties(img_index, consts.iSliderA0, consts.SLIDER_A0_INFO)

display.ShowForm(img_index, consts.iForm0)

oldval = -1
while True:
    pts = touch.GetPoints()
    if pts > 0:
        touched = display.Touched(img_index, -1)   # check widget touched
        if touched == consts.iSliderA0:
            newval = display.GetValue(img_index, consts.iSliderA0)
            if newval != oldval:
                display.SetValue(img_index, consts.iGaugeB0, newval)
                display.Show(img_index, consts.iGaugeB0)
                oldval = newval
    # small sleep to yield (utime.sleep_ms)

#cleanup
#config.cleanup(display, touch)
touch.close()
display.close()