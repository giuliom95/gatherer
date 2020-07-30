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

	Mat4f persp()
	{
		const float a = 1 / tanf(fov);
		const float d = zfar - znear;
		const float b = -(zfar + znear) / d;
		const float c = -2 * zfar * znear / d;
		return {
			a, 0, 0, 0,
			0, a, 0, 0,
			0, 0, b, c,
			0, 0, 1, 0
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
		//BOOST_LOG_TRIVIAL(info) << pos[0] << " " << pos[1] << " " << pos[2];
		const Vec3f z = normalize(-1*lpos);
		const Vec3f up{0,1,0};
		const Vec3f x = normalize(cross(up, z));
		const Vec3f y = cross(z, x);
		mrot = Mat4f{-1*x, y, z, {}};
		pos = lpos + focus;
	}
};

#endif