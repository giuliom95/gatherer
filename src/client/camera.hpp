#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include "math.hpp"

class Camera
{
public:
	Vec3f focus;
	float pitch;
	float yaw;
	float r;

	float fov;
	float zfar;
	float znear;

	float aspect;

	Mat4f w2c()
	{
		Mat4f mrot;
		Vec3f pos;
		viewmatrices(pos, mrot);
		return translationMatrix(-1*pos)*transpose(mrot);
	}

	Mat4f c2w()
	{
		Mat4f mrot;
		Vec3f pos;
		viewmatrices(pos, mrot);
		return mrot*translationMatrix(pos);
	}

	Vec3f eye()
	{
		Mat4f mrot;
		Vec3f pos;
		viewmatrices(pos, mrot);
		return pos;
	}

	Mat4f persp()
	{
		const float tg = tanf(PI_OVER_180*fov);
		const float sx = 2 / (aspect*tg);
		const float sy = 2 / tg;

		const float det = zfar - znear;
		const float c = - (zfar + znear) / det;
		const float d = - (2*zfar*znear) / det;

		return {
			sx,  0,  0, 0,
			 0, sy,  0, 0,
			 0,  0,  c, d,
			 0,  0, -1, 0
		};
	}
private:
	void viewmatrices(Vec3f& pos, Mat4f& mrot)
	{
		const float rad_pitch = pitch * PI_OVER_180;
		const float rad_yaw   = yaw   * PI_OVER_180;
		const float a = r * cosf(rad_pitch);
		const Vec3f lpos{
			a*cosf(rad_yaw), 
			-r*sinf(rad_pitch), 
			a*sinf(rad_yaw)
		};
		//LOG(info) << pitch;
		const Vec3f z = normalize(-1*lpos);
		const Vec3f up{0,1,0};
		const Vec3f x = normalize(cross(up, z));
		const Vec3f y = cross(z, x);
		mrot = Mat4f{x, y, z, {}};
		pos = lpos + focus;
	}
};

#endif