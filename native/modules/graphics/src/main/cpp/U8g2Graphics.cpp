/*-
 * ========================START=================================
 * UCGDisplay :: Native :: Graphics
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
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <memory>

#include <UcgdConfig.h>
#include <Global.h>
#include <Common.h>

#include <U8g2Graphics.h>
#include <U8g2Hal.h>
#include <U8g2Utils.h>
#include <ServiceLocator.h>
#include <DeviceManager.h>
#include <exception>

#if (defined(__arm__) || defined(__aarch64__)) && defined(__linux__)

#include <ProviderManager.h>
#include <UcgdCperipheryProvider.h>
#include <UcgdPigpioProvider.h>
#include <UcgdPigpiodProvider.h>
#include <UcgdLibgpiodProvider.h>

#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

std::stringstream outputBuffer;

void clearOutputBuffer() {
    outputBuffer.str(std::string());
}

void writeOutputBuffer(const char *output) {
    if (output != nullptr) {
        outputBuffer << std::string(output);
    }
}

std::string readOutputBuffer() {
    return std::string(outputBuffer.str());
}

void uncaught_exception_handler() {
    Log &log = ServiceLocator::getInstance().getLogger();
    log.debug("An uncaught exception has been thrown from the native library. Terminating program");
    std::cerr << Backtrace() << std::endl;
    abort(); //sigabrt
}

void signal_handler(int sig) {
    g_SignalStatus = sig;
}

void registerSignalHandlers() {
#if !defined(_WIN32) && !defined(_WIN64)
    struct sigaction act{};
    memset(&act, '\0', sizeof(act));

    //Register our signal handler(s)
    act.sa_handler = &signal_handler;
    act.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&act.sa_mask);

    sigaction(SIGTERM, &act, nullptr);
    sigaction(SIGINT, &act, nullptr);
    sigaction(SIGABRT, &act, nullptr);
    //sigaction(SIGSEGV, &act, nullptr);
#endif
}

void setFontFlag(JNIEnv *env, jlong id, bool value) {
    std::shared_ptr<ucgd_t> info = ServiceLocator::getInstance().getDeviceManager()->getDevice(static_cast<uintptr_t>(id));
    if (info == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Unable to set font flag. Device Info for address does not exist");
        return;
    }
    info->flag_font = value;
}

bool getFontFlag(JNIEnv *env, jlong id) {
    std::shared_ptr<ucgd_t> info = ServiceLocator::getInstance().getDeviceManager()->getDevice(
            static_cast<uintptr_t>(id));
    if (info == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Unable to set font flag. Device Info for address does not exist");
        return false;
    }
    return info->flag_font;
}

bool checkValidity(JNIEnv *env, jlong id) {
    if (ServiceLocator::getInstance().getDeviceManager()->getDevice(static_cast<uintptr_t>(id)) == nullptr) {
        JNI_ThrowNativeLibraryException(env, std::string("Invalid Id specified (") + std::to_string(id) + std::string(")"));
        return false;
    }
    return true;
}

void updateKeyValueStore(JNIEnv *env, const std::string &key, jobject &value, option_map_t &map) {
    jclass clsString = env->FindClass(CLS_STRING);
    jclass clsObj = env->GetObjectClass(value);
    jclass clsInteger = env->FindClass(CLS_INTEGER);
    jclass clsBoolean = env->FindClass(CLS_BOOLEAN);
    jclass clsNativeUtils = env->FindClass(CLS_NativeUtils);
    jmethodID midToInteger = env->GetStaticMethodID(clsNativeUtils, "toInteger", "(Ljava/lang/Object;)I");
    jmethodID midToString = env->GetMethodID(clsString, "toString", "()Ljava/lang/String;");
    jmethodID midToBoolean = env->GetMethodID(clsBoolean, "booleanValue", "()Z");

    if (clsObj == nullptr) {
        map.insert(std::make_pair(key, nullptr));
    }
    else if (env->IsInstanceOf(value, clsString)) {
        auto strValue = (jstring) env->CallObjectMethod(value, midToString);
        map.insert(std::make_pair(key, std::string(env->GetStringUTFChars(strValue, 0))));
    }
    else if (env->IsInstanceOf(value, clsInteger)) {
        jint intValue = env->CallStaticIntMethod(clsNativeUtils, midToInteger, value);
        map.insert(std::make_pair(key, intValue));
    }
    else if (env->IsInstanceOf(value, clsBoolean)) {
        bool boolValue = (bool) env->CallBooleanMethod(value, midToBoolean);
        map.insert(std::make_pair(key, boolValue));
    }
    else {
        throw std::runtime_error(std::string("Unsupported value type for key: \"") + key + std::string("\""));
    }
}

void processOptions(JNIEnv *env, jobject options, option_map_t &map, std::unique_ptr<Log> &log) {
    jclass clsMap = env->GetObjectClass(options);
    jmethodID midEntrySet = env->GetMethodID(clsMap, "entrySet", "()Ljava/util/Set;");

    jclass clsEntrySet = env->FindClass(CLS_SET);
    jmethodID midIterator = env->GetMethodID(clsEntrySet, "iterator", "()Ljava/util/Iterator;");

    jclass clsIterator = env->FindClass(CLS_ITERATOR);
    jmethodID midHasNext = env->GetMethodID(clsIterator, "hasNext", "()Z");
    jmethodID midNext = env->GetMethodID(clsIterator, "next", "()Ljava/lang/Object;");

    jclass clsEntry = env->FindClass(CLS_MAP_ENTRY);
    jmethodID midGetKey = env->GetMethodID(clsEntry, "getKey", "()Ljava/lang/Object;");
    jmethodID midGetValue = env->GetMethodID(clsEntry, "getValue", "()Ljava/lang/Object;");

    jclass clsString = env->FindClass(CLS_STRING);
    jmethodID midToString = env->GetMethodID(clsString, "toString", "()Ljava/lang/String;");

    jobject objEntrySet = env->CallObjectMethod(options, midEntrySet);
    jobject objIterator = env->CallObjectMethod(objEntrySet, midIterator);

    bool hasNext = (bool) env->CallBooleanMethod(objIterator, midHasNext);

    log->debug("processOptions() : Extracting all available options from the map");

    int ctr = 1;
    while (hasNext) {
        jobject objEntry = env->CallObjectMethod(objIterator, midNext);

        //Get key and value
        jobject objKey = env->CallObjectMethod(objEntry, midGetKey);
        jobject objValue = env->CallObjectMethod(objEntry, midGetValue);

        //convert key and value to strings
        auto jstrKey = (jstring) env->CallObjectMethod(objKey, midToString);
        auto jstrValue = (jstring) env->CallObjectMethod(objValue, midToString);

        const char *strKey = env->GetStringUTFChars(jstrKey, 0);
        const char *strValue = env->GetStringUTFChars(jstrValue, 0);

        log->debug("processOptions() : {}) Key = {}, Value = {}", ctr++, std::string(strKey), std::string(strValue));

        updateKeyValueStore(env, std::string(strKey), objValue, map);
        hasNext = (bool) env->CallBooleanMethod(objIterator, midHasNext);
    }

    log->debug("processOptions() : Processed a total of {} option entries", map.size());
}

void copyToBgraBufferHorizontal(int width, const std::shared_ptr<ucgd_t> &context) {
    Log &log = ServiceLocator::getInstance().getLogger();
    uint8_t *u8g2Buffer = context->buffer;
    uint8_t *bgraBuffer = context->bufferBgra;

    unsigned int primary = context->primary_color;
    unsigned int secondary = context->secondary_color;

    unsigned int pBlue = ((primary >> 24) & 0xff);
    unsigned int pGreen = ((primary >> 16) & 0xff);
    unsigned int pRed = ((primary >> 8) & 0xff);
    unsigned int pAlpha = (primary & 0xff);

    unsigned int sBlue = ((secondary >> 24) & 0xff);
    unsigned int sGreen = ((secondary >> 16) & 0xff);
    unsigned int sRed = ((secondary >> 8) & 0xff);
    unsigned int sAlpha = (secondary & 0xff);

    int bpos = 0;
    for (int i = 0; i < context->bufferSize; i++) {
        uint8_t data = *(u8g2Buffer + i);
        //read from msb to lsb
        for (int pos = 7; pos >= 0; pos--) {
            if (data & (1 << pos)) {
                //primary color (Set bit)
                bgraBuffer[bpos + 0] = pBlue;   //blue
                bgraBuffer[bpos + 1] = pGreen;   //green
                bgraBuffer[bpos + 2] = pRed;   //red
                bgraBuffer[bpos + 3] = pAlpha; //alpha
            }
            else {
                //secondary color (Clear bit)
                bgraBuffer[bpos + 0] = sBlue; //blue
                bgraBuffer[bpos + 1] = sGreen; //green
                bgraBuffer[bpos + 2] = sRed; //red
                bgraBuffer[bpos + 3] = sAlpha; //alpha
            }
            bpos += 4;
        }
    }
}

void copyToBgraBufferVertical(int width, const std::shared_ptr<ucgd_t> &context) {
    const std::unique_ptr<DeviceManager> &devMgr = ServiceLocator::getInstance().getDeviceManager();
    Log &log = ServiceLocator::getInstance().getLogger();

    uint8_t *u8g2Buffer = context->buffer;
    uint8_t *bgraBuffer = context->bufferBgra;

    unsigned int primary = context->primary_color;
    unsigned int secondary = context->secondary_color;

    unsigned int pBlue = ((primary >> 24) & 0xff);
    unsigned int pGreen = ((primary >> 16) & 0xff);
    unsigned int pRed = ((primary >> 8) & 0xff);
    unsigned int pAlpha = (primary & 0xff);

    unsigned int sBlue = ((secondary >> 24) & 0xff);
    unsigned int sGreen = ((secondary >> 16) & 0xff);
    unsigned int sRed = ((secondary >> 8) & 0xff);
    unsigned int sAlpha = (secondary & 0xff);

    int bitpos = 0, x = 0, y = 0, page = 0, pos = 0, mark = 0, bpos = 0;
    //note: 1 page = width (bytes), e.g. for a 128x64 display, we have 128 bytes per page.
    //total number of pages can be calulated by dividing the display height with 8. (ex: 128x64 display have 8 pages in total)
    while (true) {
        if (x > (width - 1)) {
            //are we at the last bit?
            if (bitpos++ >= 7) {
                if (pos >= context->bufferSize)
                    break;
                page++;
                bitpos = 0;
                pos = width * page;
                mark = pos;
            }
            else {
                pos = mark;
            }
            x = 0;
        }

        uint8_t data = u8g2Buffer[pos++];
        y = (page * 8) + bitpos;
        unsigned int bit = (data & (1 << bitpos)) != 0 ? 1 : 0;
        if (bit) {
            //Primary color (Set bit)
            bgraBuffer[bpos + 0] = pBlue;   //blue
            bgraBuffer[bpos + 1] = pGreen;   //green
            bgraBuffer[bpos + 2] = pRed;   //red
            bgraBuffer[bpos + 3] = pAlpha; //alpha
        }
        else {
            //Secondary color (Clear bit)
            bgraBuffer[bpos + 0] = sBlue; //blue
            bgraBuffer[bpos + 1] = sGreen; //green
            bgraBuffer[bpos + 2] = sRed; //red
            bgraBuffer[bpos + 3] = sAlpha; //alpha
        }
        //log.debug("Bit = {}, Idx = {}, Page = {} ({}, {}) = {}", bitpos, pos - 1, page, x, y, bit);
        x++;
        bpos += 4;
    }
}

/**
* Convert u8g2 buffer to bgra buffer
*/
void updateBgraBuffer(jlong id) {
    const std::unique_ptr<DeviceManager> &devMgr = ServiceLocator::getInstance().getDeviceManager();
    const std::shared_ptr<ucgd_t> &context = devMgr->getDevice(static_cast<uintptr_t>(id));
    Log &log = ServiceLocator::getInstance().getLogger();

    //do not update buffer if not in virtual mode
    if (!context->flag_virtual)
        return;

    //auto& log = ServiceLocator::getInstance().getLogger();
    uint8_t *u8g2Buffer = context->buffer;
    uint8_t *bgraBuffer = context->bufferBgra;

    if (u8g2Buffer == nullptr || bgraBuffer == nullptr || context->bufferSize <= 0 || context->bufferBgraSize <= 0)
        return;

    int width = context->u8g2->pixel_buf_width;
    int height = context->u8g2->pixel_buf_height;

    //u8g2_ll_hvline_vertical_top_lsb
    //u8g2_ll_hvline_horizontal_right_lsb
    if (context->u8g2->ll_hvline == u8g2_ll_hvline_vertical_top_lsb) {
        copyToBgraBufferVertical(width, context);
    }
    else {
        copyToBgraBufferHorizontal(width, context);
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {

    registerSignalHandlers();

    //Load global references
    JNI_Load(jvm);

    JNIEnv *env;
    jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION);

    //Unaught exception handler
    std::set_terminate(uncaught_exception_handler);

    //Initialize Utils
    U8gUtils_Load(env);

    //Initialize HAL
    U8g2Hal_Init();

    return JNI_VERSION;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    JNI_Unload(vm);
}

jlong Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setup(JNIEnv *env, jclass cls, jstring setupProc, jint commInt, jint commType, jint rotation, jintArray pin_config, jobject buffer, jobject bufferBgra, jobject options, jboolean virtualMode, jobject logger, jstring version) {
    jobject globalLogger;
    JNI_MakeGlobal(env, logger, globalLogger);

    std::unique_ptr<Log> log = std::make_unique<Log>(globalLogger);

    if (buffer == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Pixel buffer not provided");
        return -1;
    }

    if (virtualMode && bufferBgra == nullptr) {
        JNI_ThrowNativeLibraryException(env, "BGRA Pixel buffer not provided");
        return -1;
    }

    std::string sVersion;
    if (version != nullptr) {
        sVersion = std::string(env->GetStringUTFChars(version, nullptr));
    }

    log->debug("=========================================================================================================");
    log->debug(" ");
    log->debug("██╗   ██╗ ██████╗ ██████╗ ██████╗ ██╗███████╗██████╗ ██╗      █████╗ ██╗   ██╗");
    log->debug("██║   ██║██╔════╝██╔════╝ ██╔══██╗██║██╔════╝██╔══██╗██║     ██╔══██╗╚██╗ ██╔╝");
    log->debug("██║   ██║██║     ██║  ███╗██║  ██║██║███████╗██████╔╝██║     ███████║ ╚████╔╝");
    log->debug("██║   ██║██║     ██║   ██║██║  ██║██║╚════██║██╔═══╝ ██║     ██╔══██║  ╚██╔╝");
    log->debug("╚██████╔╝╚██████╗╚██████╔╝██████╔╝██║███████║██║     ███████╗██║  ██║   ██║");
    log->debug(" ╚═════╝  ╚═════╝ ╚═════╝ ╚═════╝ ╚═╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝");
    log->debug(" ");
    log->debug("=========================================================================================================");
    log->debug("Native Library - Version {}", sVersion.c_str());
    log->debug("=========================================================================================================");

    std::string setup_proc_name;
    if (setupProc != nullptr) {
        setup_proc_name = std::string(env->GetStringUTFChars(setupProc, nullptr));
    }
    else {
        JNI_ThrowNativeLibraryException(env, "Setup procedure name cannot be null");
        return -1;
    }

    //1. Setup procedure should not be empty
    if (setup_proc_name.empty()) {
        JNI_ThrowNativeLibraryException(env, "Setup procedure name cannot be empty");
        return -1;
    }

    //2. Verify pin mapping
    if (pin_config == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Pin map not specified");
        return -1;
    }

    //3. Verify pin config array length (should be 16)
    jsize len = env->GetArrayLength(pin_config);
    if (len != 16) {
        JNI_ThrowNativeLibraryException(env, std::string("Pin map array should be exactly 16 of length (Actual: ") + std::to_string(len) + std::string(")"));
        return -1;
    }

    int tmp[len];
    JNI_CopyJIntArray(env, pin_config, tmp, len);

    //convert to struct
    auto *pinMap = reinterpret_cast<u8g2_pin_map_t *>(tmp);

    //5. Make sure options is not null
    if (options == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Missing options");
        return -1;
    }

    //6. Iterate all available properties in options map and store it into a temporary std::map
    option_map_t mapOptions;
    processOptions(env, options, mapOptions, log);

    //4. Verify that the rotation number is within the allowed range
    if (rotation < 0 || rotation > 4) {
        JNI_ThrowNativeLibraryException(env, std::string("Invalid rotation (") + std::to_string(rotation) + ")");
        return -1;
    }

    //Get actual rotation value
    const u8g2_cb_t *_rotation = U8g2Util_ToRotation(rotation);

    static bool initialized;

    //Initialize service locator and providers
    ServiceLocator &locator = ServiceLocator::getInstance();

    if (!initialized) {
        //Initialize Logger
        locator.setLogger(log);

        //Initialize Device Manager
        locator.setDeviceManager(std::make_unique<DeviceManager>());

#if (defined(__arm__) || defined(__aarch64__)) && defined(__linux__)
        if (mapOptions[OPT_EXTRA_DEBUG_INFO].has_value()) {
            g_ShowExtraDebugInfo = std::any_cast<bool>(mapOptions[OPT_EXTRA_DEBUG_INFO]);
        }
        //Initialize Providers
        locator.setProviderManager(std::make_unique<ProviderManager>());
        auto &pMan = locator.getProviderManager();

        std::string pigAddr = mapOptions[OPT_PIGPIO_ADDR].has_value() ? std::any_cast<std::string>(mapOptions[OPT_PIGPIO_ADDR]) : "";
        std::string pigPort = mapOptions[OPT_PIGPIO_PORT].has_value() ? std::any_cast<std::string>(mapOptions[OPT_PIGPIO_PORT]) : "";

        //Register supported providers
        if (!pMan->isRegistered(PROVIDER_CPERIPHERY))
            pMan->registerProvider(std::make_shared<UcgdCperipheryProvider>());
        if (!pMan->isRegistered(PROVIDER_PIGPIO))
            pMan->registerProvider(std::make_shared<UcgdPigpioProvider>());
        if (!pMan->isRegistered(PROVIDER_PIGPIOD))
            pMan->registerProvider(std::make_shared<UcgdPigpiodProvider>(pigAddr, pigPort));
        if (!pMan->isRegistered(PROVIDER_LIBGPIOD))
            pMan->registerProvider(std::make_shared<UcgdLibgpiodProvider>());
#endif
        initialized = true;
    }
    //7. Setup and Initialize the Display
    try {
        locator.getLogger().debug("setup() : Converting direct buffer to native buffer");
        auto *pixelBuffer = static_cast<uint8_t *>(env->GetDirectBufferAddress(buffer));
        std::shared_ptr<ucgd_t> &context = U8g2Util_SetupAndInitDisplay(setup_proc_name, commInt, commType, _rotation,
                                                                        *pinMap, mapOptions, pixelBuffer, virtualMode);
        context->buffer = pixelBuffer;
        context->bufferSize = env->GetDirectBufferCapacity(buffer);
        locator.getLogger().debug("setup() : Pixel buffer initialized with size {}", context->bufferSize);
        if (virtualMode && bufferBgra != nullptr) {
            context->bufferBgra = static_cast<uint8_t *>(env->GetDirectBufferAddress(bufferBgra));
            context->bufferBgraSize = env->GetDirectBufferCapacity(bufferBgra);
        }
        locator.getLogger().debug("setup() : Returning to java land");
        return context->address();
    } catch (std::exception &e) {
        JNI_ThrowNativeLibraryException(env, std::string("Failed to initialize the display device. Reason: \"") +
                                             std::string(e.what()) + std::string("\""));
    }
    return -1;
}

//long id, int x, int y, int width, int height
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawBox(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawBox(toU8g2(id), static_cast <u8g2_uint_t>(x), static_cast <u8g2_uint_t>(y),
                     static_cast <u8g2_uint_t>(width), static_cast <u8g2_uint_t>(height));
    END_CATCH
}

//long id, int x, int y, int count, int height, byte[] bitmap
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawBitmap(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint count, jint height, jbyteArray bitmap) {
    if (!checkValidity(env, id))
        return;
    if (bitmap == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Bitmap data cannot be null");
        return;
    }
    BEGIN_CATCH
        jsize len = env->GetArrayLength(bitmap);
        uint8_t tmp[len];
        JNI_CopyJByteArray(env, bitmap, tmp, len);
        u8g2_DrawBitmap(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                        static_cast<u8g2_uint_t>(count), static_cast<u8g2_uint_t>(height), tmp);
    END_CATCH
}

//long id, int x, int y, int radius, int options
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawCircle(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint radius, jint options) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawCircle(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                        static_cast<u8g2_uint_t>(radius), static_cast<uint8_t>(options));
    END_CATCH
}

//long id, int x, int y, int radius, int options
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawDisc(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint radius, jint options) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawDisc(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                      static_cast<u8g2_uint_t>(radius), static_cast<uint8_t>(options));
    END_CATCH
}

//long id, int x, int y, int rx, int ry, int options
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawEllipse(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint rx, jint ry, jint options) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawEllipse(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y), static_cast<u8g2_uint_t>(rx), static_cast<u8g2_uint_t>(ry), static_cast<uint8_t>(options));
    END_CATCH
}

//long id, int x, int y, int rx, int ry, int options
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawFilledEllipse(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint rx, jint ry, jint options) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawFilledEllipse(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y), static_cast<u8g2_uint_t>(rx), static_cast<u8g2_uint_t>(ry), static_cast<uint8_t>(options));
    END_CATCH
}

//long id, int x, int y, int width, int height
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawFrame(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawFrame(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                       static_cast<u8g2_uint_t>(width), static_cast<u8g2_uint_t>(height));
    END_CATCH
}

//long id, int x, int y, short encoding
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawGlyph(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jshort encoding) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawGlyph(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                       static_cast<uint16_t>(encoding));
    END_CATCH
}

//long id, int x, int y, int width
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawHLine(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawHLine(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                       static_cast<u8g2_uint_t>(width));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawVLine(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawVLine(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                       static_cast<u8g2_uint_t>(width));
    END_CATCH
}

//long id, int x, int y, int x1, int y1
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawLine(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint x1, jint y1) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawLine(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                      static_cast<u8g2_uint_t>(x1), static_cast<u8g2_uint_t>(y1));
    END_CATCH
}

//long id, int x, int y
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawPixel(JNIEnv *env, jclass cls, jlong id, jint x, jint y) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawPixel(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y));
    END_CATCH
}

//long id, int x, int y, int width, int height, int radius
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawRoundedBox(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height, jint radius) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawRBox(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                      static_cast<u8g2_uint_t>(width), static_cast<u8g2_uint_t>(height),
                      static_cast<u8g2_uint_t>(radius));
    END_CATCH
}

//long id, int x, int y, int width, int height, int radius
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawRoundedFrame(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height, jint radius) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawRFrame(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y),
                        static_cast<u8g2_uint_t>(width), static_cast<u8g2_uint_t>(height),
                        static_cast<u8g2_uint_t>(radius));
    END_CATCH
}

//long id, int x, int y, String value
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawString(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jstring value) {
    if (value == nullptr) {
        JNI_ThrowNativeLibraryException(env, "drawString() : Value is null");
        return;
    }
    if (!checkValidity(env, id))
        return;
    if (!getFontFlag(env, id)) {
        JNI_ThrowNativeLibraryException(env, "A font needs to be assigned prior to calling this method");
        return;
    }
    BEGIN_CATCH
        const char *c = env->GetStringUTFChars(value, nullptr);
        u8g2_DrawStr(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y), c);
    END_CATCH
}

//long id, int x0, int y0, int x1, int y1, int x2, int y2
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawTriangle(JNIEnv *env, jclass cls, jlong id, jint x0, jint y0, jint x1, jint y1, jint x2, jint y2) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_DrawTriangle(toU8g2(id), static_cast<int16_t>(x0), static_cast<int16_t>(y0), static_cast<int16_t>(x1), static_cast<int16_t>(y1), static_cast<int16_t>(x2), static_cast<int16_t>(y2));
    END_CATCH
}

//long id, int x, int y, int width, int height, byte[] data
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawXBM(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height, jbyteArray data) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        jsize len = env->GetArrayLength(data);
        uint8_t tmp[len];
        JNI_CopyJByteArray(env, data, tmp, len);
        u8g2_DrawXBM(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y), static_cast<u8g2_uint_t>(width), static_cast<u8g2_uint_t>(height), tmp);
    END_CATCH
}

//long id, int x, int y, String value
jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawUTF8(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jstring value) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        const char *c = env->GetStringUTFChars(value, nullptr);
        int retval = u8g2_DrawUTF8(toU8g2(id), static_cast<u8g2_uint_t>(x), static_cast<u8g2_uint_t>(y), c);
    END_CATCH
    return -1;
}

//long id, String text
jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getUTF8Width(JNIEnv *env, jclass cls, jlong id, jstring text) {
    if (!checkValidity(env, id))
        return -1;
    if (text == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Text cannot be null");
        return -1;
    }
    BEGIN_CATCH
        const char *c = env->GetStringUTFChars(text, nullptr);
        return u8g2_GetUTF8Width(toU8g2(id), c);
    END_CATCH
    return -1;
}

//long id, byte[] data
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFont__J_3B(JNIEnv *env, jclass cls, jlong id, jbyteArray data) {
    if (!checkValidity(env, id))
        return;
    if (data == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Font data cannot be null");
        return;
    }

    BEGIN_CATCH
        jsize len = env->GetArrayLength(data);
        if (len <= 0) {
            JNI_ThrowNativeLibraryException(env, "Invalid font data");
            return;
        }
        uint8_t tmp[len];
        JNI_CopyJByteArray(env, data, tmp, len);
        u8g2_SetFont(toU8g2(id), tmp);
        setFontFlag(env, id, true);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFont__JLjava_lang_String_2(JNIEnv *env, jclass cls, jlong id, jstring fontName) {
    if (!checkValidity(env, id))
        return;
    if (fontName == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Font key cannot be null");
        return;
    }
    BEGIN_CATCH
        std::string font = std::string(env->GetStringUTFChars(fontName, nullptr));
        uint8_t *fontData = U8g2hal_GetFontByName(font);
        if (fontData == nullptr) {
            JNI_ThrowNativeLibraryException(env, std::string("Unable to retrieve font data for: ") + font);
            return;
        }
        u8g2_SetFont(toU8g2(id), fontData);
        setFontFlag(env, id, true);
    END_CATCH
}

//long id, int mode
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontMode(JNIEnv *env, jclass cls, jlong id, jint mode) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontMode(toU8g2(id), static_cast<uint8_t>(mode));
    END_CATCH
}

//long id, int direction
void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontDirection(JNIEnv *env, jclass cls, jlong id, jint mode) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontDirection(toU8g2(id), static_cast<uint8_t>(mode));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontPosBaseline(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontPosBaseline(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontPosBottom(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontPosBottom(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontPosTop(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontPosTop(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontPosCenter(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontPosCenter(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontRefHeightAll(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontRefHeightAll(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontRefHeightExtendedText(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontRefHeightExtendedText(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFontRefHeightText(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFontRefHeightText(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setFlipMode(JNIEnv *env, jclass cls, jlong id, jboolean enable) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetFlipMode(toU8g2(id), enable);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setPowerSave(JNIEnv *env, jclass cls, jlong id, jboolean enable) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetPowerSave(toU8g2(id), enable);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setDrawColor(JNIEnv *env, jclass cls, jlong id, jint color) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetDrawColor(toU8g2(id), static_cast<uint8_t>(color));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_initDisplay(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_InitDisplay(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_firstPage(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_FirstPage(toU8g2(id));
    END_CATCH
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_nextPage(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_NextPage(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getAscent(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetAscent(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getDescent(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetDescent(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getMaxCharWidth(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetMaxCharWidth(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getMaxCharHeight(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetMaxCharHeight(toU8g2(id));
    END_CATCH
    return -1;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_sendBuffer(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SendBuffer(toU8g2(id));
        updateBgraBuffer(id);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_clearBuffer(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_ClearBuffer(toU8g2(id));
        updateBgraBuffer(id);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_clearDisplay(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_ClearDisplay(toU8g2(id));
        updateBgraBuffer(id);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_begin(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_t *u8g2 = toU8g2(id);
        u8g2_InitDisplay(u8g2);
        u8g2_ClearDisplay(u8g2);
        u8g2_SetPowerSave(u8g2, 0);
    END_CATCH
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getHeight(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetDisplayHeight(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getWidth(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetDisplayWidth(toU8g2(id));
    END_CATCH
    return -1;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_clear(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        //Process: home(); clearDisplay(); clearBuffer();
        u8g2_t *u8g2 = toU8g2(id);
        //home (not implemented here)
        u8g2_ClearDisplay(u8g2);
        u8g2_ClearBuffer(u8g2);
    END_CATCH
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setAutoPageClear(JNIEnv *env, jclass cls, jlong id, jint clear) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_SetAutoPageClear(toU8g2(id), clear);
    END_CATCH
    return -1;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setBitmapMode(JNIEnv *env, jclass cls, jlong id, jint mode) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetBitmapMode(toU8g2(id), static_cast<uint8_t>(mode));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setContrast(JNIEnv *env, jclass cls, jlong id, jint value) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetContrast(toU8g2(id), value);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setDisplayRotation(JNIEnv *env, jclass cls, jlong id, jint rotation) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_cb_t *_rotation = U8g2Util_ToRotation(rotation);
        if (_rotation == nullptr)
            return;
        u8g2_SetDisplayRotation(toU8g2(id), _rotation);
    END_CATCH
}

jbyteArray Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getBuffer(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return nullptr;

    BEGIN_CATCH
        u8g2_t *ptr = toU8g2(id);
        uint8_t *buffer = u8g2_GetBufferPtr(ptr);
        int width = u8g2_GetBufferTileWidth(ptr);
        int height = u8g2_GetBufferTileHeight(ptr);
        int size = 8 * (width * height);

        jbyteArray arr = env->NewByteArray(size);
        env->SetByteArrayRegion(arr, 0, size, reinterpret_cast<jbyte *>(buffer));
        return arr;
    END_CATCH
    return nullptr;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getBufferTileWidth(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetBufferTileWidth(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getBufferTileHeight(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetBufferTileHeight(toU8g2(id));
    END_CATCH
    return -1;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setBufferCurrTileRow(JNIEnv *env, jclass cls, jlong id, jint row) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetBufferCurrTileRow(toU8g2(id), static_cast<uint8_t>(row));
    END_CATCH
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getBufferCurrTileRow(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        return u8g2_GetBufferCurrTileRow(toU8g2(id));
    END_CATCH
    return -1;
}

jint Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_getStrWidth(JNIEnv *env, jclass cls, jlong id, jstring text) {
    if (!checkValidity(env, id))
        return -1;
    BEGIN_CATCH
        const char *c = env->GetStringUTFChars(text, nullptr);
        return u8g2_GetStrWidth(toU8g2(id), c);
    END_CATCH
    return -1;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setClipWindow(JNIEnv *env, jclass cls, jlong id, jint x0, jint y0, jint x1, jint y1) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetClipWindow(toU8g2(id), x0, y0, x1, y1);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setMaxClipWindow(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_SetMaxClipWindow(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_updateDisplay__J(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_UpdateDisplay(toU8g2(id));
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_updateDisplay__JIIII(JNIEnv *env, jclass cls, jlong id, jint x, jint y, jint width, jint height) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        u8g2_UpdateDisplayArea(toU8g2(id), x, y, width, height);
    END_CATCH
}

jstring Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_exportToXBM(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return nullptr;
    BEGIN_CATCH
        clearOutputBuffer();
        std::shared_ptr<ucgd_t> context = ServiceLocator::getInstance().getDeviceManager()->getDevice(static_cast<uintptr_t>(id));
        u8g2_t* u8g2 = toU8g2(id);
        u8g2_WriteBufferXBM(u8g2, &writeOutputBuffer);
        std::string out = readOutputBuffer();
        jstring jsout = nullptr;
        if (!out.empty())
            jsout = env->NewStringUTF(out.c_str());
        return jsout;
    END_CATCH
    return nullptr;
}

jstring Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_exportToPBM(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return nullptr;
    BEGIN_CATCH
        clearOutputBuffer();
        u8g2_WriteBufferPBM(toU8g2(id), &writeOutputBuffer);
        std::string out = readOutputBuffer();
        jstring jsout = nullptr;
        if (!out.empty())
            jsout = env->NewStringUTF(out.c_str());
        return jsout;
    END_CATCH
    return nullptr;
}

jstring Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_exportToXBM2(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return nullptr;
    BEGIN_CATCH
        clearOutputBuffer();
        u8g2_WriteBufferXBM2(toU8g2(id), &writeOutputBuffer);
        std::string out = readOutputBuffer();
        jstring jsout = nullptr;
        if (!out.empty())
            jsout = env->NewStringUTF(out.c_str());
        return jsout;
    END_CATCH
    return nullptr;
}

jstring Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_exportToPBM2(JNIEnv *env, jclass cls, jlong id) {
    if (!checkValidity(env, id))
        return nullptr;
    BEGIN_CATCH
        clearOutputBuffer();
        u8g2_WriteBufferPBM2(toU8g2(id), &writeOutputBuffer);
        std::string out = readOutputBuffer();
        jstring jsout = nullptr;
        if (!out.empty())
            jsout = env->NewStringUTF(out.c_str());
        return jsout;
    END_CATCH
    return nullptr;
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_sendCommand(JNIEnv *env, jclass cls, jlong id, jstring fmt, jbyteArray args) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        const char *c = env->GetStringUTFChars(fmt, nullptr);
        jsize len = env->GetArrayLength(args);
        uint8_t tmp[len];
        JNI_CopyJByteArray(env, args, tmp, len);
        u8g2_SendF(toU8g2(id), c, tmp);
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setPrimaryColor(JNIEnv *env, jclass cls, jlong id, jint color) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        std::shared_ptr<ucgd_t> context = ServiceLocator::getInstance().getDeviceManager()->getDevice(static_cast<uintptr_t>(id));
        context->primary_color = color;
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_setSecondaryColor(JNIEnv *env, jclass cls, jlong id, jint color) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        std::shared_ptr<ucgd_t> context = ServiceLocator::getInstance().getDeviceManager()->getDevice(static_cast<uintptr_t>(id));
        context->secondary_color = color;
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawPixels(JNIEnv *env, jclass cls, jlong id, jbyteArray buffer, jint size) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        jsize bufferSize = env->GetArrayLength(buffer);
        uint8_t tmp[bufferSize];
        JNI_CopyJByteArray(env, buffer, tmp, bufferSize);
        u8g2_t *u8g2 = toU8g2(id);
        int x = 0, y = 0;
        int width = u8g2_GetBufferTileWidth(u8g2) * 8;
        int height = u8g2_GetBufferTileHeight(u8g2) * 8;
        u8g2_ClearBuffer(u8g2);
        for (int i = 0; i < bufferSize; i++) {
            uint8_t data = tmp[i];
            //start with the most significant bit
            for (int pos = 7; pos >= 0; pos++) {
                if (x >= width) {
                    x = 0;
                    y++;
                }
                //if bit is set, draw
                if (((1 << pos) & data) != 0)
                    u8g2_DrawPixel(u8g2, x, y);
                x++;
            }
        }
    END_CATCH
}

void Java_com_ibasco_ucgdisplay_core_u8g2_U8g2Graphics_drawPixelsBgra(JNIEnv *env, jclass cls, jlong id, jbyteArray buffer, jint size) {
    if (!checkValidity(env, id))
        return;
    BEGIN_CATCH
        jsize jBufferSize = env->GetArrayLength(buffer);
        jbyte *jBuffer = env->GetByteArrayElements(buffer, nullptr);
        u8g2_t *u8g2 = toU8g2(id);
        int width = u8g2_GetBufferTileWidth(u8g2) * 8;
        int height = u8g2_GetBufferTileHeight(u8g2) * 8;
        u8g2_ClearBuffer(u8g2);
        int x = 0, y = 0;
        for (int i = 0; i < jBufferSize; i += 4) {
            if (x >= width) {
                y++;
                x = 0;
            }
            auto blue = static_cast<uint8_t>(jBuffer[i + 0]);
            auto green = static_cast<uint8_t>(jBuffer[i + 1]);
            auto red = static_cast<uint8_t>(jBuffer[i + 2]);
            auto alpha = static_cast<uint8_t>(jBuffer[i + 3]);
            int color = (blue << 24) | (green << 16) | (red << 8) | (alpha);
            //std::cout << "X = " << std::to_string(x) << ", Y = " << std::to_string(y) << " = " << std::to_string(color) << std::endl;
            if (color != 0)
                u8g2_DrawPixel(u8g2, x, y);
            x++;
        }
        env->ReleaseByteArrayElements(buffer, jBuffer, 0);
    END_CATCH
}

#pragma clang diagnostic pop
