package com.ibasco.pidisplay.drivers.clcd.enums;

import com.pi4j.io.gpio.PinState;

public enum LcdReadWriteState {
    READ(PinState.HIGH),
    WRITE(PinState.LOW);

    private PinState state;

    LcdReadWriteState(PinState state) {
        this.state = state;
    }

    public PinState getPinState() {
        return state;
    }
}