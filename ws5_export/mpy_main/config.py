# config.py
import graphics4d as gfx
#from consts import consts as consts
consts = {"PROJECT_ORIENTATION": "PORTRAIT_R",
          "iForm0": 0,
          "iSliderA0": 1,
          "iSliderA0Fill": 2,
          "iSliderA0Thumb": 3,
          "iGaugeB0": 4,
          "SLIDER_A0_INFO": [50],
          }

def start():
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
    return display, touch#, img_index

def cleanup(display, touch):
    touch.close()
    display.close()