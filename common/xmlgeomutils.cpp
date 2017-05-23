// Copyright 2013 Trimble Navigation Limited. All Rights Reserved.

#include "./xmlgeomutils.h"
//#include <stdlib.h>
#include <iostream>

namespace XmlGeomUtils {

// Misc Utilities--------------------------------------
const double EqualTol = 1.0e-3;
const double EqualTolSq = EqualTol * EqualTol;

bool NormalEqual(const CPoint3d &a, const CPoint3d &b, const CPoint3d &c, const CVector3d &normal) {
	//std::cout << a.x() << " " << a.y() << " " << a.z() << std::endl;
	//std::cout << b.x() << " " << b.y() << " " << b.z() << std::endl;
	//std::cout << c.x() << " " << c.y() << " " << c.z() << std::endl;

	CVector3d a_b = CVector3d(b.x() - a.x(), b.y() - a.y(), b.z() - a.z());
	CVector3d a_c = CVector3d(c.x() - a.x(), c.y() - a.y(), c.z() - a.z());
	CVector3d my_normal = a_b % a_c;
	return my_normal.DirectionEqual(normal);
}

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

bool CVector3d::DirectionEqual(const CVector3d& vec)
{
  auto dir = (*this^vec)/std::sqrtf(*this^*this)/std::sqrtf(vec^vec);
  //std::cout <<"cos angle "<< dir << std::endl;
  return dir>0 ? true :false;
}

CVector3d CVector3d::operator+(const CVector3d& vec) const {
  return CVector3d(x_ + vec.x_, y_ + vec.y_, z_ + vec.z_);
}

CVector3d CVector3d::operator-(const CVector3d& vec) const {
  return CVector3d(x_ - vec.x_, y_ - vec.y_, z_ - vec.z_);
}

CVector3d CVector3d::operator%(const CVector3d& vec) const{
  return CVector3d (y_ * vec.z_ - z_ * vec.y_,
                    z_ * vec.x_ - x_ * vec.z_,
                    x_ * vec.y_ - y_ * vec.x_);
}

float CVector3d::operator^(const CVector3d& vec) const{
  return (x_ * vec.x_ + y_ * vec.y_ + z_ * vec.z_);
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
