package com.ibasco.pidisplay.drivers.glcd;

import com.ibasco.pidisplay.drivers.glcd.enums.GlcdCommInterface;
import com.ibasco.pidisplay.drivers.glcd.enums.GlcdControllerType;
import com.ibasco.pidisplay.drivers.glcd.enums.GlcdSize;
import org.slf4j.Logger;

import java.util.ArrayList;
import java.util.List;

import static org.slf4j.LoggerFactory.getLogger;

/**
 * Holds important meta data information on a Graphics Display
 *
 * @author Rafael Ibasco
 */
@SuppressWarnings({"unused", "WeakerAccess"})
public class GlcdDisplay {

    public static final Logger log = getLogger(GlcdDisplay.class);

    private String name;
    private GlcdControllerType controller;
    private GlcdSetupInfo[] setupDetails;
    private GlcdSize displaySize;

    GlcdDisplay(GlcdControllerType controller, String name, int tileWidth, int tileHeight, GlcdSetupInfo... setupInfo) {
        this.name = name;
        this.controller = controller;
        this.setupDetails = setupInfo;
        this.displaySize = GlcdSize.get(tileWidth, tileHeight);
    }

    public boolean hasCommType(GlcdCommInterface protocol) {
        for (GlcdSetupInfo info : setupDetails) {
            if ((info.getProtocols() & protocol.getValue()) > 0)
                return true;
        }
        return false;
    }

    public List<GlcdCommInterface> getCommTypes() {
        if (setupDetails == null || setupDetails.length <= 0)
            return null;
        List<GlcdCommInterface> protocols = new ArrayList<>();
        for (GlcdSetupInfo setup : setupDetails) {
            for (GlcdCommInterface protocol : GlcdCommInterface.values()) {
                if ((setup.getProtocols() & protocol.getValue()) > 0)
                    protocols.add(protocol);
            }
        }
        return protocols;
    }

    public GlcdSize getDisplaySize() {
        return displaySize;
    }

    public GlcdControllerType getController() {
        return controller;
    }

    public String getName() {
        return name;
    }

    public GlcdSetupInfo[] getSetupDetails() {
        return setupDetails;
    }
}
