import graphics4d

# Initialize display
display = graphics4d.Graphics4D()
display.initialize()

# Basic drawing
display.cls()
display.rectangle(10, 10, 100, 100, graphics4d.RED)
display.rectangle_filled(120, 10, 220, 100, graphics4d.GREEN)
display.circle(300, 200, 50, graphics4d.BLUE)
display.line(0, 0, 400, 300, graphics4d.YELLOW)

# Set backlight
display.set_backlight(500)  # Medium brightness

# Load and display images
img_handle = display.load_image("image.gci")
display.show_image(img_handle, 0)