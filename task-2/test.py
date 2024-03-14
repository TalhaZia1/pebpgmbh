import RPi.GPIO as GPIO
import time

# GPIO Pin Numbering is not actual
# ToDo: Adding correct Pin Numbering as per Raspberry Pi Hardware

# Define Hardware GPIO pin mappings for connectors
pin_mappings = {
    'connector_1': [1, 2, 3, 4, 5, 6],
    'connector_2': [7, 8, 9, 10, 11, 12],
    'connector_3': [13, 14, 15, 16, 17, 18]
}

# Set up OUTPUT LED for User
OUTPUT_LED = 19

# Define feedback pins for connectors
feedback_pins = {
    'connector_1': 20,
    'connector_2': 21,
    'connector_3': 22
}

# 1 to 22 GPIO pins for demonstration
# ToDo: To be updated as per Hardware

GPIO.setup(OUTPUT_LED, GPIO.OUT)

# Set up switches
for feedback_pin in feedback_pins.values():
    GPIO.setup(feedback_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

# Waiting for Feedback for all Connectors Feedback (Checking if all connectors are connected)
while not all(GPIO.input(pin) for pin in feedback_pins.values()):
    time.sleep(1)

# Define the correspondence between connector 1 and connector 2 pins
# Pin index Numbering from 0 to 5 
connector_1_to_2_mapping = {
    0: [0, 4],      # CAN High
    1: [1, 2],      # CAN Low
    2: [2, 3],      # LV 12V
    3: [3, 5],      # Light Out (Gnd)
    4: [4, 0],      # HV 42V
    5: [5, 1]       # Gnd 
}

# Define the correspondence between connector 1 and connector 3 pins
# Pin index Numbering from 0 to 5 
connector_1_to_3_mapping = {
    0: [1, 3],      # CAN High 
    1: [2, 5],      # CAN Low
    2: [3, 0],      # LV 12V
    3: [4, 2],      # Light Out (Gnd)
    4: [5, 1],      # HV 42V
    5: [6, 4]       # Gnd 
}

# Function to test connector correspondence
def test_correspondence(connector_pins, mapping):
    for i, output_pin in enumerate(connector_pins):
        GPIO.setup(output_pin, GPIO.OUT, initial=GPIO.LOW)
        GPIO.output(output_pin, GPIO.HIGH)
        
        # Check input pins with corresponding pins based on mapping
        for j, input_pin in enumerate(connector_pins):
            if j in mapping[i]:
                # Corresponding pin should be HIGH
                if GPIO.input(input_pin) != GPIO.HIGH:
                    return False
            else:
                # Non-corresponding pins should be LOW
                if GPIO.input(input_pin) != GPIO.LOW:
                    return False
        
        # Set output pin back to LOW
        GPIO.output(output_pin, GPIO.LOW)
    
    return True

# Test correspondence for connector 2
connector_2_passed = test_correspondence(pin_mappings['connector_2'], connector_1_to_2_mapping)

# Test correspondence for connector 3
connector_3_passed = test_correspondence(pin_mappings['connector_3'], connector_1_to_3_mapping)

# Set Output LED based on test results
if connector_2_passed and connector_3_passed:
    GPIO.output(OUTPUT_LED, GPIO.HIGH)
else:
    GPIO.output(OUTPUT_LED, GPIO.LOW)

