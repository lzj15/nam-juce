file(GLOB SRC_FILES
    "Buttons/*.png"
    "PresetManagerAssets/*.png"
    "ButtonLabels/*.png"
)

juce_add_binary_data(BinaryData SOURCES
    knob.png
    background.png
    buttonpushed.png
    buttonunpushed.png
    screens.png
    arrowexpand.png
    arrowcontract.png
    knob_minimal.png
    led_off.png
    led_on.png
    ${SRC_FILES}
)

# Need to build BinaryData with -fPIC flag on Linux
set_target_properties(BinaryData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE)
