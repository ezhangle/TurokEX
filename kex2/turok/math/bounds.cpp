// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION: Bounding box operations
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

//
// BBox_Transform
//

void BBox_Transform(bbox_t srcBox, mtx_t matrix, bbox_t *out)
{
    bbox_t box;
    vec3_t c;
    vec3_t h;
    vec3_t ct;
    vec3_t ht;
    mtx_t m;

    Vec_Copy3(box.min, srcBox.min);
    Vec_Copy3(box.max, srcBox.max);

    Mtx_Copy(m, matrix);

    Vec_Add(c, box.min, box.max);
    Vec_Scale(c, c, 0.5f);
    Vec_Sub(h, box.max, c);
    Vec_TransformToWorld(m, c, ct);

    m[ 0] = (float)fabs(m[ 0]);
    m[ 1] = (float)fabs(m[ 1]);
    m[ 2] = (float)fabs(m[ 2]);
    m[ 4] = (float)fabs(m[ 4]);
    m[ 5] = (float)fabs(m[ 5]);
    m[ 6] = (float)fabs(m[ 6]);
    m[ 8] = (float)fabs(m[ 8]);
    m[ 9] = (float)fabs(m[ 9]);
    m[10] = (float)fabs(m[10]);
    m[12] = 0;
    m[13] = 0;
    m[14] = 0;

    Vec_TransformToWorld(m, h, ht);

    Vec_Sub(box.min, ct, ht);
    Vec_Add(box.max, ct, ht);

    Vec_Copy3(out->min, box.min);
    Vec_Copy3(out->max, box.max);
}

//
// kexBBox::kexBBox
//

kexBBox::kexBBox(void) {
    Clear();
}

//
// kexBBox::kexBBox
//

kexBBox::kexBBox(const kexVec3 &vMin, const kexVec3 &vMax) {
    this->min = vMin;
    this->max = vMax;
}

//
// kexBBox::Clear
//

void kexBBox::Clear(void) {
    min.Clear();
    max.Clear();
}

//
// kexBBox::Center
//

kexVec3 kexBBox::Center(void) const {
    return kexVec3(
        (max.x + min.x) * 0.5f,
        (max.y + min.y) * 0.5f,
        (max.z + min.z) * 0.5f);
}

//
// kexBBox::Radius
//

float kexBBox::Radius(void) const {
    return (max - Center()).Unit();
}

//
// kexBBox::PointInside
//

bool kexBBox::PointInside(const kexVec3 &vec) const {
    return !(vec[0] < min[0] || vec[1] < min[1] || vec[2] < min[2] ||
             vec[0] > max[0] || vec[1] > max[1] || vec[2] > max[2]);
}

//
// kexBBox::IntersectingBox
//

bool kexBBox::IntersectingBox(const kexBBox &box) const {
    return !(box.max[0] < min[0] || box.max[1] < min[1] || box.max[2] < min[2] ||
             box.min[0] > max[0] || box.min[1] > max[1] || box.min[2] > max[2]);
}

//
// kexBBox::operator+
//

kexBBox kexBBox::operator+(const float radius) const {
    kexVec3 vmin = min;
    kexVec3 vmax = max;

    vmin.x -= radius;
    vmin.y -= radius;
    vmin.z -= radius;

    vmax.x += radius;
    vmax.y += radius;
    vmax.z += radius;

    return kexBBox(vmin, vmax);
}

//
// kexBBox::operator|
//

kexBBox kexBBox::operator|(const kexMatrix &matrix) const {
    kexVec3 c  = Center();
    kexVec3 ct = c | matrix;
    
    kexMatrix mtx(matrix);
    
    for(int i = 0; i < 3; i++) {
        mtx.vectors[i].x = (float)fabs(mtx.vectors[i].x);
        mtx.vectors[i].y = (float)fabs(mtx.vectors[i].y);
        mtx.vectors[i].z = (float)fabs(mtx.vectors[i].z);
    }
    
    kexVec3 ht = c | mtx;
    
    return kexBBox(ct - ht, ct + ht);
}

//
// kexBBox::operator|=
//

kexBBox &kexBBox::operator|=(const kexMatrix &matrix) {
    kexVec3 c  = Center();
    kexVec3 ct = c | matrix;
    
    kexMatrix mtx(matrix);
    
    for(int i = 0; i < 3; i++) {
        mtx.vectors[i].x = (float)fabs(mtx.vectors[i].x);
        mtx.vectors[i].y = (float)fabs(mtx.vectors[i].y);
        mtx.vectors[i].z = (float)fabs(mtx.vectors[i].z);
    }
    
    kexVec3 ht = c | mtx;
    
    min = (ct - ht);
    max = (ct + ht);
    
    return *this;
}

//
// kexBBox::operator=
//

kexBBox &kexBBox::operator=(const kexBBox &bbox) {
    min = bbox.min;
    max = bbox.max;

    return *this;
}

//
// kexBBox:LineIntersect
//

bool kexBBox::LineIntersect(const kexVec3 &start, const kexVec3 &end) {
    float ld[3];
    kexVec3 center = Center();
    kexVec3 extents = max - center;
    kexVec3 lineDir = (end - start) * 0.5f;
    kexVec3 lineCenter = lineDir + start;
    kexVec3 dir = lineCenter - center;

    ld[0] = (float)fabs(lineDir[0]);
    if((float)fabs(dir[0]) > extents[0] + ld[0]) {
        return false;
    }

    ld[1] = (float)fabs(lineDir[1]);
    if((float)fabs(dir[1]) > extents[1] + ld[1]) {
        return false;
    }

    ld[2] = (float)fabs(lineDir[2]);
    if((float)fabs(dir[2]) > extents[2] + ld[2]) {
        return false;
    }

    kexVec3 cross = lineDir.Cross(dir);

    if((float)fabs(cross[0]) > extents[1] * ld[2] + extents[2] * ld[1]) {
        return false;
    }

    if((float)fabs(cross[1]) > extents[0] * ld[2] + extents[2] * ld[0]) {
        return false;
    }

    if((float)fabs(cross[2]) > extents[0] * ld[1] + extents[1] * ld[0]) {
        return false;
    }

    return true;
}
