/*-
 * ========================START=================================
 * Organization: Universal Character/Graphics display library
 * Project: UCGDisplay :: Native Library
 * Filename: Global.cpp
 * 
 * ---------------------------------------------------------
 * %%
 * Copyright (C) 2018 Universal Character/Graphics display library
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

#include "Global.h"
#include <iostream>
#include <vector>

JavaVM *cachedJVM;

jclass clsNativeLibraryException;

static vector<jclass> globalRefClasses;

void JNI_Load(JavaVM *jvm) {
    cachedJVM = jvm;
    JNIEnv *env;
    jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION);
    JNI_MakeGlobal(env, CLS_NativeLibraryException, clsNativeLibraryException);
}

void JNI_Unload(JavaVM *vm) {
    JNIEnv *env;
    vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION);
    for (auto it : globalRefClasses) {
        env->DeleteGlobalRef(it);
    }
    globalRefClasses.clear();
}

void JNI_GetEnv(JNIEnv *env) {
    if (cachedJVM == nullptr) {
        cerr << "VM not yet cached" << endl;
        return;
    }
    cachedJVM->GetEnv((void **) &env, JNI_VERSION);
}

void JNI_MakeGlobal(JNIEnv *env, const char *name, jclass &cls) {
    jclass tmp;
    tmp = env->FindClass(name);
    cls = (jclass) env->NewGlobalRef(tmp);
    globalRefClasses.emplace_back(cls);
    env->DeleteLocalRef(tmp);
}

void JNI_ThrowIOException(JNIEnv *env, string msg) {
    jclass clsEx = env->FindClass(CLS_IOEXCEPTION);
    env->ThrowNew(clsEx, msg.c_str());
}

void JNI_ThrowNativeLibraryException(JNIEnv *env, string msg) {
    env->ThrowNew(clsNativeLibraryException, msg.c_str());
}

void JNI_CopyJByteArray(JNIEnv *env, jbyteArray arr, uint8_t *buffer, int length) {
    if (length <= 0) {
        JNI_ThrowNativeLibraryException(env, "Invalid array length");
        return;
    }
    if (buffer == nullptr) {
        JNI_ThrowNativeLibraryException(env, "Buffer is null");
        return;
    }
    jbyte *body = env->GetByteArrayElements(arr, nullptr);
    for (int i = 0; i < length; i++) {
        buffer[i] = static_cast<uint8_t>(body[i]);
    }
    env->ReleaseByteArrayElements(arr, body, 0);
}