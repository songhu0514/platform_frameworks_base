/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_HWUI_DISPLAY_LIST_H
#define ANDROID_HWUI_DISPLAY_LIST_H

#include <SkCamera.h>
#include <SkMatrix.h>

#include <utils/RefBase.h>
#include <utils/SortedVector.h>
#include <utils/String8.h>
#include <utils/Vector.h>
#include <cutils/compiler.h>

#include "utils/LinearAllocator.h"

#include "Debug.h"

#define TRANSLATION 0x0001
#define ROTATION    0x0002
#define ROTATION_3D 0x0004
#define SCALE       0x0008
#define PIVOT       0x0010

class SkBitmap;
class SkPaint;
class SkPath;
class SkRegion;

namespace android {
namespace uirenderer {

class DeferredDisplayList;
class DisplayListOp;
class DisplayListRenderer;
class OpenGLRenderer;
class Rect;
class Layer;
class SkiaColorFilter;
class SkiaShader;

/**
 * Refcounted structure that holds data used in display list stream
 */
class DisplayListData: public LightRefBase<DisplayListData> {
public:
    LinearAllocator allocator;
    Vector<DisplayListOp*> displayListOps;
};

/**
 * Replays recorded drawing commands.
 */
class DisplayList {
public:
    DisplayList(const DisplayListRenderer& recorder);
    ANDROID_API ~DisplayList();

    // See flags defined in DisplayList.java
    enum ReplayFlag {
        kReplayFlag_ClipChildren = 0x1
    };

    status_t setViewProperties(OpenGLRenderer& renderer, Rect& dirty,
            int32_t flags, uint32_t level, DeferredDisplayList* deferredList);
    void outputViewProperties(uint32_t level);

    ANDROID_API size_t getSize();
    ANDROID_API static void destroyDisplayListDeferred(DisplayList* displayList);
    ANDROID_API static void outputLogBuffer(int fd);

    void initFromDisplayListRenderer(const DisplayListRenderer& recorder, bool reusing = false);

    status_t replay(OpenGLRenderer& renderer, Rect& dirty, int32_t flags, uint32_t level = 0,
            DeferredDisplayList* deferredList = NULL);

    void output(uint32_t level = 0);

    ANDROID_API void reset();

    void setRenderable(bool renderable) {
        mIsRenderable = renderable;
    }

    bool isRenderable() const {
        return mIsRenderable;
    }

    void setName(const char* name) {
        if (name) {
            mName.setTo(name);
        }
    }

    const char* getName() const {
        return mName.string();
    }

    void setClipChildren(bool clipChildren) {
        mClipChildren = clipChildren;
    }

    void setStaticMatrix(SkMatrix* matrix) {
        delete mStaticMatrix;
        mStaticMatrix = new SkMatrix(*matrix);
    }

    // Can return NULL
    SkMatrix* getStaticMatrix() {
        return mStaticMatrix;
    }

    void setAnimationMatrix(SkMatrix* matrix) {
        delete mAnimationMatrix;
        if (matrix) {
            mAnimationMatrix = new SkMatrix(*matrix);
        } else {
            mAnimationMatrix = NULL;
        }
    }

    void setAlpha(float alpha) {
        alpha = fminf(1.0f, fmaxf(0.0f, alpha));
        if (alpha != mAlpha) {
            mAlpha = alpha;
            mMultipliedAlpha = (int) (255 * alpha);
        }
    }

    float getAlpha() const {
        return mAlpha;
    }

    void setHasOverlappingRendering(bool hasOverlappingRendering) {
        mHasOverlappingRendering = hasOverlappingRendering;
    }

    bool hasOverlappingRendering() const {
        return mHasOverlappingRendering;
    }

    void setTranslationX(float translationX) {
        if (translationX != mTranslationX) {
            mTranslationX = translationX;
            mMatrixDirty = true;
            if (mTranslationX == 0.0f && mTranslationY == 0.0f) {
                mMatrixFlags &= ~TRANSLATION;
            } else {
                mMatrixFlags |= TRANSLATION;
            }
        }
    }

    float getTranslationX() const {
        return mTranslationX;
    }

    void setTranslationY(float translationY) {
        if (translationY != mTranslationY) {
            mTranslationY = translationY;
            mMatrixDirty = true;
            if (mTranslationX == 0.0f && mTranslationY == 0.0f) {
                mMatrixFlags &= ~TRANSLATION;
            } else {
                mMatrixFlags |= TRANSLATION;
            }
        }
    }

    float getTranslationY() const {
        return mTranslationY;
    }

    void setRotation(float rotation) {
        if (rotation != mRotation) {
            mRotation = rotation;
            mMatrixDirty = true;
            if (mRotation == 0.0f) {
                mMatrixFlags &= ~ROTATION;
            } else {
                mMatrixFlags |= ROTATION;
            }
        }
    }

    float getRotation() const {
        return mRotation;
    }

    void setRotationX(float rotationX) {
        if (rotationX != mRotationX) {
            mRotationX = rotationX;
            mMatrixDirty = true;
            if (mRotationX == 0.0f && mRotationY == 0.0f) {
                mMatrixFlags &= ~ROTATION_3D;
            } else {
                mMatrixFlags |= ROTATION_3D;
            }
        }
    }

    float getRotationX() const {
        return mRotationX;
    }

    void setRotationY(float rotationY) {
        if (rotationY != mRotationY) {
            mRotationY = rotationY;
            mMatrixDirty = true;
            if (mRotationX == 0.0f && mRotationY == 0.0f) {
                mMatrixFlags &= ~ROTATION_3D;
            } else {
                mMatrixFlags |= ROTATION_3D;
            }
        }
    }

    float getRotationY() const {
        return mRotationY;
    }

    void setScaleX(float scaleX) {
        if (scaleX != mScaleX) {
            mScaleX = scaleX;
            mMatrixDirty = true;
            if (mScaleX == 1.0f && mScaleY == 1.0f) {
                mMatrixFlags &= ~SCALE;
            } else {
                mMatrixFlags |= SCALE;
            }
        }
    }

    float getScaleX() const {
        return mScaleX;
    }

    void setScaleY(float scaleY) {
        if (scaleY != mScaleY) {
            mScaleY = scaleY;
            mMatrixDirty = true;
            if (mScaleX == 1.0f && mScaleY == 1.0f) {
                mMatrixFlags &= ~SCALE;
            } else {
                mMatrixFlags |= SCALE;
            }
        }
    }

    float getScaleY() const {
        return mScaleY;
    }

    void setPivotX(float pivotX) {
        mPivotX = pivotX;
        mMatrixDirty = true;
        if (mPivotX == 0.0f && mPivotY == 0.0f) {
            mMatrixFlags &= ~PIVOT;
        } else {
            mMatrixFlags |= PIVOT;
        }
        mPivotExplicitlySet = true;
    }

    ANDROID_API float getPivotX();

    void setPivotY(float pivotY) {
        mPivotY = pivotY;
        mMatrixDirty = true;
        if (mPivotX == 0.0f && mPivotY == 0.0f) {
            mMatrixFlags &= ~PIVOT;
        } else {
            mMatrixFlags |= PIVOT;
        }
        mPivotExplicitlySet = true;
    }

    ANDROID_API float getPivotY();

    void setCameraDistance(float distance) {
        if (distance != mCameraDistance) {
            mCameraDistance = distance;
            mMatrixDirty = true;
            if (!mTransformCamera) {
                mTransformCamera = new Sk3DView();
                mTransformMatrix3D = new SkMatrix();
            }
            mTransformCamera->setCameraLocation(0, 0, distance);
        }
    }

    float getCameraDistance() const {
        return mCameraDistance;
    }

    void setLeft(int left) {
        if (left != mLeft) {
            mLeft = left;
            mWidth = mRight - mLeft;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    float getLeft() const {
        return mLeft;
    }

    void setTop(int top) {
        if (top != mTop) {
            mTop = top;
            mHeight = mBottom - mTop;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    float getTop() const {
        return mTop;
    }

    void setRight(int right) {
        if (right != mRight) {
            mRight = right;
            mWidth = mRight - mLeft;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    float getRight() const {
        return mRight;
    }

    void setBottom(int bottom) {
        if (bottom != mBottom) {
            mBottom = bottom;
            mHeight = mBottom - mTop;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    float getBottom() const {
        return mBottom;
    }

    void setLeftTop(int left, int top) {
        if (left != mLeft || top != mTop) {
            mLeft = left;
            mTop = top;
            mWidth = mRight - mLeft;
            mHeight = mBottom - mTop;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    void setLeftTopRightBottom(int left, int top, int right, int bottom) {
        if (left != mLeft || top != mTop || right != mRight || bottom != mBottom) {
            mLeft = left;
            mTop = top;
            mRight = right;
            mBottom = bottom;
            mWidth = mRight - mLeft;
            mHeight = mBottom - mTop;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    void offsetLeftRight(float offset) {
        if (offset != 0) {
            mLeft += offset;
            mRight += offset;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    void offsetTopBottom(float offset) {
        if (offset != 0) {
            mTop += offset;
            mBottom += offset;
            if (mMatrixFlags > TRANSLATION && !mPivotExplicitlySet) {
                mMatrixDirty = true;
            }
        }
    }

    void setCaching(bool caching) {
        mCaching = caching;
    }

    int getWidth() {
        return mWidth;
    }

    int getHeight() {
        return mHeight;
    }

private:
    void init();

    void clearResources();

    void updateMatrix();

    class TextContainer {
    public:
        size_t length() const {
            return mByteLength;
        }

        const char* text() const {
            return (const char*) mText;
        }

        size_t mByteLength;
        const char* mText;
    };

    Vector<SkBitmap*> mBitmapResources;
    Vector<SkBitmap*> mOwnedBitmapResources;
    Vector<SkiaColorFilter*> mFilterResources;

    Vector<SkPaint*> mPaints;
    Vector<SkPath*> mPaths;
    SortedVector<SkPath*> mSourcePaths;
    Vector<SkRegion*> mRegions;
    Vector<SkMatrix*> mMatrices;
    Vector<SkiaShader*> mShaders;
    Vector<Layer*> mLayers;

    sp<DisplayListData> mDisplayListData;

    size_t mSize;

    bool mIsRenderable;
    uint32_t mFunctorCount;

    String8 mName;

    // View properties
    bool mClipChildren;
    float mAlpha;
    int mMultipliedAlpha;
    bool mHasOverlappingRendering;
    float mTranslationX, mTranslationY;
    float mRotation, mRotationX, mRotationY;
    float mScaleX, mScaleY;
    float mPivotX, mPivotY;
    float mCameraDistance;
    int mLeft, mTop, mRight, mBottom;
    int mWidth, mHeight;
    int mPrevWidth, mPrevHeight;
    bool mPivotExplicitlySet;
    bool mMatrixDirty;
    bool mMatrixIsIdentity;
    uint32_t mMatrixFlags;
    SkMatrix* mTransformMatrix;
    Sk3DView* mTransformCamera;
    SkMatrix* mTransformMatrix3D;
    SkMatrix* mStaticMatrix;
    SkMatrix* mAnimationMatrix;
    bool mCaching;
}; // class DisplayList

}; // namespace uirenderer
}; // namespace android

#endif // ANDROID_HWUI_OPENGL_RENDERER_H
