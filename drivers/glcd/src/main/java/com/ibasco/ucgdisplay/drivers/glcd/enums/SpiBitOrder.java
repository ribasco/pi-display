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
package com.ibasco.ucgdisplay.drivers.glcd.enums;

import com.ibasco.ucgdisplay.drivers.glcd.GlcdOptionValue;

/**
 * SPI bit Order enumeration
 *
 * @author Rafael Ibasco
 */
public enum SpiBitOrder implements GlcdOptionValue<Integer> {
    MSB_FIRST(0),
    LSB_FIRST(1);

    private int value;

    SpiBitOrder(int value) {
        this.value = value;
    }

    @Override
    public Integer toValue() {
        return value;
    }

    @Override
    public Class<Integer> getType() {
        return Integer.class;
    }
}
