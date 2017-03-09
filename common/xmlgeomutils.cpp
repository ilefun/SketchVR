// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include "./xmlgeomutils.h"


namespace XmlGeomUtils {

// Misc Utilities--------------------------------------
const double EqualTol = 1.0e-3;
const double EqualTolSq = EqualTol * EqualTol;

inline bool AreEqual(double val1, double val2, double tol = EqualTol) {
  double diff = val1 - val2;
  return diff <= tol && diff >= -tol;
}

void CPoint3d::Transform(double matrix[16])
{
    double a, b, c, w;

    a = x_ * matrix[0] + y_ * matrix[4] + z_ * matrix[8] + matrix[12];
    b = x_ * matrix[1] + y_ * matrix[5] + z_ * matrix[9] + matrix[13];
    c = x_ * matrix[2] + y_ * matrix[6] + z_ * matrix[10] + matrix[14];
    w = x_ * matrix[3] + y_ * matrix[7] + z_ * matrix[11] + matrix[15];

    SetLocation(a/w,b/w,c/w);
}

// Point Class----------------------------------------
void CPoint3d::operator+=(const CVector3d& vec) {
  x_ += vec.x();
  y_ += vec.y();
  z_ += vec.z();
}

void CPoint3d::operator-=(const CVector3d& vec) {
  x_ -= vec.x();
  y_ -= vec.y();
  z_ -= vec.z();
}

CPoint3d CPoint3d::operator+(const CVector3d& vec) const {
  double x = x_ + vec.x();
  double y = y_ + vec.y();
  double z = z_ + vec.z();
  return CPoint3d(x, y, z);
}

CPoint3d CPoint3d::operator+(const CPoint3d& pt) const {
  double x = x_ + pt.x_;
  double y = y_ + pt.y_;
  double z = z_ + pt.z_;
  return CPoint3d(x, y, z);
}

CPoint3d CPoint3d::operator-(const CVector3d& vec) const {
  double x = x_ - vec.x();
  double y = y_ - vec.y();
  double z = z_ - vec.z();
  return CPoint3d(x, y, z);
}

CVector3d CPoint3d::operator-(const CPoint3d& pt) const {
  double dx = x_ - pt.x_;
  double dy = y_ - pt.y_;
  double dz = z_ - pt.z_;
  return CVector3d(dx, dy, dz);
}

bool CPoint3d::operator==(const CPoint3d& v) const {
  return AreEqual(x_, v.x_, EqualTol) && 
         AreEqual(y_, v.y_, EqualTol) &&
         AreEqual(z_, v.z_, EqualTol);
}

bool CPoint3d::operator!=(const CPoint3d& v) const {
  return !operator==(v);
}

// Vector Class----------------------------------------
void CVector3d::Transform(double matrix[16])
{
    double a, b, c;

    a = x_ * matrix[0] + y_ * matrix[4] + z_ * matrix[8];
    b = x_ * matrix[1] + y_ * matrix[5] + z_ * matrix[9];
    c = x_ * matrix[2] + y_ * matrix[6] + z_ * matrix[10];

    SetDirection(a,b,c);
}

CVector3d CVector3d::operator+(const CVector3d& vec) const {
  return CVector3d(x_ + vec.x_, y_ + vec.y_, z_ + vec.z_);
}

CVector3d CVector3d::operator-(const CVector3d& vec) const {
  return CVector3d(x_ - vec.x_, y_ - vec.y_, z_ - vec.z_);
}

void CVector3d::operator+=(const CVector3d& vec) {
  x_ += vec.x_;
  y_ += vec.y_;
  z_ += vec.z_;
}

void CVector3d::operator-=(const CVector3d& vec) {
  x_ -= vec.x_;
  y_ -= vec.y_;
  z_ -= vec.z_;
}

CVector3d CVector3d::operator*(double scale) const {
  return CVector3d(x_ * scale, y_ * scale, z_ * scale);
}

CVector3d CVector3d::operator/(double scale) const {
  return CVector3d(x_ / scale, y_ / scale, z_ / scale);
}

void CVector3d::operator*=(double scale) {
  x_ *= scale;
  y_ *= scale;
  z_ *= scale;
}

void CVector3d::operator/=(double scale) {
  x_ /= scale;
  y_ /= scale;
  z_ /= scale;
}

bool CVector3d::operator==(const CVector3d& v) const {
  return AreEqual(x_, v.x_, EqualTol) && 
         AreEqual(y_, v.y_, EqualTol) &&
         AreEqual(z_, v.z_, EqualTol);
}

bool CVector3d::operator!=(const CVector3d& v) const {
  return !operator==(v);
}

} // end namespace XmlGeomUtils
