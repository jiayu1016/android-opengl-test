# Copyright 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgles3jni
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := gles3jni.cpp \
		image.c \
				   RendererES2.cpp \
				   RendererES3.cpp \
				   my_Renderer.cpp
LOCAL_STATIC_LIBRARIES := libpng
LOCAL_LDLIBS    := -llog -lGLESv3 -lEGL -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libpng
LOCAL_SRC_FILES = libpng/png.c \
		  libpng/pngerror.c \
		  libpng/pngget.c \
		  libpng/pngmem.c \
		  libpng/pngpread.c \
		  libpng/pngread.c \
		  libpng/pngrio.c \
		  libpng/pngrtran.c \
		  libpng/pngrutil.c \
		  libpng/pngset.c \
		  libpng/pngtrans.c \
		  libpng/pngwio.c \
		  libpng/pngwrite.c \
		  libpng/pngwtran.c \
		  libpng/pngwutil.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS := -lz

include $(BUILD_STATIC_LIBRARY)
