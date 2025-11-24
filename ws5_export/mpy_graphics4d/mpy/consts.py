# consts.py
import graphics4d as gfx

#%% declare Variables from consts.h up until SetupMedia(){}, PROJECT_ORIENTATION should be given as string
PROJECT_ORIENTATION = "PORTRAIT_R"
iForm0 = 0
iSliderA0 = 1
iSliderA0Fill = 2
iSliderA0Thumb = 3
iGaugeB0 = 4

SLIDER_A0_INFO = [50]

#%% Set Values from inside SetupMedia(){}
def setup_media(display, hndl):
    display.SetProperties(hndl, iSliderA0, SLIDER_A0_INFO)
    return None


#%% dont change starting and cleanup routines
def start(enable_touch=True):
    # create instance
    display = gfx.init()
    # set up display
    ORIENTATIONS = {"PORTRAIT": 0,
                    "LANDSCAPE": 1,
                    "PORTRAIT_R": 2,
                    "LANDSCAPE_R": 3}
    display.Screenmode(ORIENTATIONS[PROJECT_ORIENTATION])
    display.Contrast(8)
    # initialize touch if needed
    if enable_touch:
        touch = gfx.Touch4D()
    # Load embedded gcx like original SetupMedia()
    hndl = display.LoadImageControl(gfx.GRAPHICS_GCX)
    if not hndl:
        display.ClearScreen()
        display.print("Failed to initialize media\n")
        display.Contrast(15)
        while True:
            pass
    setup_media(display, hndl)
    return display, hndl, touch

def cleanup(display, touch):
    touch.close()
    display.close()
    return None