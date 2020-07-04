/*-
 * ========================START=================================
 * UCGDisplay :: Graphics LCD driver
 * %%
 * Copyright (C) 2018 - 2020 Universal Character/Graphics display library
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU General Lesser Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/lgpl-3.0.html>.
 * =========================END==================================
 */
package com.ibasco.ucgdisplay.drivers.glcd;

import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdBufferLayout;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdCommProtocol;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdController;
import com.ibasco.ucgdisplay.drivers.glcd.enums.GlcdSize;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * Holds important meta data information of a Graphics display device
 *
 * @author Rafael Ibasco
 */
public class GlcdDisplay {
    private final String name;
    private final GlcdController controller;
    private final GlcdSetupInfo[] setupDetails;
    private final GlcdSize displaySize;
    private final GlcdBufferLayout bufferType;

    GlcdDisplay(GlcdController controller, String name, int tileWidth, int tileHeight, GlcdBufferLayout bufferType, GlcdSetupInfo... setupInfo) {
        this.name = name;
        this.controller = controller;
        this.bufferType = bufferType;
        this.setupDetails = setupInfo;
        this.displaySize = GlcdSize.get(tileWidth, tileHeight);
        assert displaySize != null;
    }

    /**
     * Check if this display supports the provided bus interface
     *
     * @param busInterface
     *         The {@link GlcdCommProtocol} to check
     *
     * @return <code>True</code> if this display supports the provided bus interface
     */
    public boolean hasBusInterface(GlcdCommProtocol busInterface) {
        for (GlcdSetupInfo info : setupDetails) {
            if ((info.getProtocols() & busInterface.getValue()) > 0)
                return true;
        }
        return false;
    }

    /**
     * @return A list of supported bus interfaces of this display
     */
    public List<GlcdCommProtocol> getBusInterfaces() {
        if (setupDetails == null || setupDetails.length <= 0)
            return null;
        List<GlcdCommProtocol> protocols = new ArrayList<>();
        for (GlcdSetupInfo setup : setupDetails) {
            for (GlcdCommProtocol protocol : GlcdCommProtocol.values()) {
                if ((setup.getProtocols() & protocol.getValue()) > 0)
                    protocols.add(protocol);
            }
        }
        return protocols;
    }

    /**
     * @return A {@link GlcdSize} containing information about the display's dimensions
     */
    public GlcdSize getDisplaySize() {
        return displaySize;
    }

    /**
     * @return A {@link GlcdController} describing the controller of this display
     */
    public GlcdController getController() {
        return controller;
    }

    /**
     * @return Returns the name of the display (excluding controller name)
     */
    public String getName() {
        return name;
    }

    /**
     * @return Returns the buffer type/layout of the display
     */
    public GlcdBufferLayout getBufferType() {
        return bufferType;
    }

    public GlcdSetupInfo[] getSetupDetails() {
        return setupDetails;
    }

    @Override
    public String toString() {
        return getController().name() + " :: " + getName();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        GlcdDisplay that = (GlcdDisplay) o;
        return name.equals(that.name) &&
                controller.equals(that.controller) &&
                displaySize.equals(that.displaySize) &&
                bufferType.equals(that.bufferType);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, controller, displaySize, bufferType);
    }
}
