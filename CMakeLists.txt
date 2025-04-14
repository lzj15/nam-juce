cmake_minimum_required(VERSION 3.28.1)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(neural-amp-modeler VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(deps/JUCE)
add_subdirectory(deps/NeuralAmpModelerCore/Dependencies/eigen)

juce_add_plugin(${PROJECT_NAME}
    PRODUCT_NAME "NAM"
    VERSION ${PLUGIN_VERSION}
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        deps/NeuralAmpModelerCore/NAM
        deps/NeuralAmpModelerCore/Dependencies/nlohmann
        deps/AudioDSPTools/dsp
        deps/AudioDSPTools/dsp/ResamplingContainer
)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/NeuralAmpModeler.cpp
        src/StatusedTrigger.cpp
        src/ToneStack.cpp
        src/PluginEditor.cpp
        src/PluginProcessor.cpp
        deps/NeuralAmpModelerCore/NAM/activations.cpp
        deps/NeuralAmpModelerCore/NAM/convnet.cpp
        deps/NeuralAmpModelerCore/NAM/dsp.cpp
        deps/NeuralAmpModelerCore/NAM/get_dsp.cpp
        deps/NeuralAmpModelerCore/NAM/lstm.cpp
        deps/NeuralAmpModelerCore/NAM/util.cpp
        deps/NeuralAmpModelerCore/NAM/wavenet.cpp
        deps/AudioDSPTools/dsp/dsp.cpp
        deps/AudioDSPTools/dsp/ImpulseResponse.cpp
        deps/AudioDSPTools/dsp/NoiseGate.cpp
        deps/AudioDSPTools/dsp/RecursiveLinearFilter.cpp
        deps/AudioDSPTools/dsp/wav.cpp
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        NAM_SAMPLE_FLOAT
        DSP_SAMPLE_FLOAT
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        Eigen3::Eigen
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
