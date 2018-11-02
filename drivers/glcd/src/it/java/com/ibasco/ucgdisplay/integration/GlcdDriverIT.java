package com.ibasco.ucgdisplay.integration;

import com.ibasco.ucgdisplay.drivers.glcd.Glcd;
import com.ibasco.ucgdisplay.drivers.glcd.GlcdConfig;
import com.ibasco.ucgdisplay.drivers.glcd.GlcdConfigBuilder;
import com.ibasco.ucgdisplay.drivers.glcd.GlcdDisplay;
import com.ibasco.ucgdisplay.drivers.glcd.GlcdDriver;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdBusInterface;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdControllerType;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdSize;
import com.ibasco.ucgdisplay.drivers.glcd.exceptions.GlcdConfigException;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.function.Executable;

/**
 * Graphics display driver integration test
 *
 * @author Rafael Ibasco
 */
class GlcdDriverIT {

    private GlcdDriver driver;

    private Executable createVirtualDriverExecutable(GlcdConfig config) {
        return () -> driver = new GlcdDriver(config, true);
    }

    @Test
    @DisplayName("Test basic virtual driver construction with no args")
    void testBasicVirtualDriverNoArgs() {
        GlcdConfig config = GlcdConfigBuilder.create().build();
        assertThrows(GlcdConfigException.class, createVirtualDriverExecutable(config));
    }

    @Test
    @DisplayName("Test basic construction of virtual glcd driver")
    void testBasicVirtualDriverConstruction() {
        GlcdConfig config = GlcdConfigBuilder
                .create()
                .display(Glcd.ST7920.D_128x64)
                .busInterface(GlcdBusInterface.SPI_HW_4WIRE_ST7920)
                .build();

        assertDoesNotThrow(createVirtualDriverExecutable(config));
        assertNotNull(driver);
        assertTrue(driver.getId() > 0);
        assertNotNull(driver.getConfig());
        GlcdDisplay display = driver.getConfig().getDisplay();

        assertNotNull(display);
        assertEquals(GlcdSize.SIZE_128x64, display.getDisplaySize());
        assertEquals(GlcdControllerType.ST7920, display.getController());
        assertTrue(display.getSetupDetails().length > 0);
        assertEquals("u8g2_Setup_st7920_s_128x64_f", config.getSetupProcedure());
    }

    @Test
    @DisplayName("Test driver construction with a non-supported bus interface")
    void testNonSupportedProtocol() {
        GlcdConfig config = GlcdConfigBuilder.create()
                .display(Glcd.ST7920.D_128x64)
                .busInterface(GlcdBusInterface.I2C_HW)
                .build();

        assertThrows(GlcdConfigException.class, createVirtualDriverExecutable(config));
    }

    @Test
    @DisplayName("Test construction of virtual driver with invalid configuration")
    void testInvalidConfigVirtualDriver() {
        GlcdConfig config = GlcdConfigBuilder
                .create()
                .display(Glcd.ST7920.D_128x64)
                .busInterface(GlcdBusInterface.SPI_SW_4WIRE)
                .build();
        assertThrows(GlcdConfigException.class, createVirtualDriverExecutable(config));
    }
}