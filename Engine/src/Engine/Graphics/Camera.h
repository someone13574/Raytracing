#pragma once

#include "Matrix.h"

namespace Graphics
{
	class Camera
	{
	public:
		Camera() {};
		Camera(double x, double y, double z) : position(x, y, z), targetPosition(0, 0, 0){ }
	public:
		void UpdateCameraToWorldMatrix()
		{
			Vector3<double> cw = targetPosition - position;
			cw.Normalize();
			Vector3<double> cu = Vector3<double>::Cross(cw, Vector3<double>(0, 1, 0));
			cu.Normalize();
			Vector3<double> cv = Vector3<double>::Cross(cu, cw);
			cv.Normalize();
			cameraToWorldMatrix = ::Matrix3x3<double>(cu, cv, -cw);
		}
	public:
		::Matrix3x3<double> cameraToWorldMatrix = ::Matrix3x3<double>(0, 0, 0, 0, 0, 0, 0, 0, 0);
		Vector3<double> targetPosition = Vector3<double>(0, 0, 0);
		Vector3<double> position = Vector3<double>(-1, 0, 0);
	};
}