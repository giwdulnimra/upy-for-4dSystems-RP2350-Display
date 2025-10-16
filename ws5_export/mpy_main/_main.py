# main.py
import graphics4d as gfx
import consts

# call startup-routine
display, hndl_index, touch = consts.start(enable_touch=True)

# Show First Form
display.ShowForm(hndl_index, consts.iForm0)
oldval = -1
while True:
    pts = touch.GetPoints()
    if pts > 0:
        touched = display.Touched(hndl_index, -1)   # check widget touched
        if touched == consts.iSliderA0:
            newval = display.GetValue(hndl_index, consts.iSliderA0)
            if newval != oldval:
                display.SetValue(hndl_index, consts.iGaugeB0, newval)
                display.Show(hndl_index, consts.iGaugeB0)
                oldval = newval
    # small sleep to yield (utime.sleep_ms)

# call cleanup-routine
config.cleanup(display, touch)