package com.ibasco.pidisplay.drivers.clcd.exceptions;

import com.ibasco.pidisplay.drivers.clcd.enums.LcdPin;

public class LcdPinNotMappedException extends RuntimeException {

    private LcdPin pin;

    public LcdPinNotMappedException(LcdPin pin) {
        super(String.format("Pin '%s' is not yet mapped", pin.getName()));
        this.pin = pin;
    }

    public LcdPin getPin() {
        return pin;
    }
}