/*-
 * ========================START=================================
 * UCGDisplay :: Graphics LCD driver
 * %%
 * Copyright (C) 2018 - 2021 Universal Character/Graphics display library
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

import com.ibasco.ucgdisplay.core.u8g2.U8g2ByteEvent;
import com.ibasco.ucgdisplay.core.u8g2.U8g2ByteEventListener;
import com.ibasco.ucgdisplay.core.u8g2.U8g2GpioEvent;
import com.ibasco.ucgdisplay.core.u8g2.U8g2GpioEventListener;

/**
 * Interface for handling U8g2 display events.
 *
 * @author Rafael Ibasco
 */
public interface GlcdDriverEventHandler extends U8g2ByteEventListener, U8g2GpioEventListener {
    @Override
    void onByteEvent(U8g2ByteEvent event);

    @Override
    default void onGpioEvent(U8g2GpioEvent event) {
        //optional only
    }
}
