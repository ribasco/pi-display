package com.ibasco.pidisplay.core.u8g2;

@FunctionalInterface
public interface U8g2GpioEventListener {
    void onGpioEvent(U8g2GpioEvent event);
}
