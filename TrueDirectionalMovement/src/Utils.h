#pragma once

#include "Offsets.h"

#define PI 3.1415926535897932f
#define TWOTHIRDS_PI 2.0943951023931955f
#define TWO_PI 6.2831853071795865f
#define PI2 1.5707963267948966f
#define PI3 1.0471975511965977f
#define PI4 0.7853981633974483f
#define PI8 0.3926990816987242f

struct AngleZX
{
	double z;
	double x;
	double distance;
};

void GetAngle(const ConsoleRE::NiPoint3& a_from, const ConsoleRE::NiPoint3& a_to, AngleZX& angle);
bool GetAngle(ConsoleRE::TESObjectREFR* a_target, AngleZX& angle);
ConsoleRE::NiPoint3 GetCameraPos();
float NormalAbsoluteAngle(float a_angle);
float NormalRelativeAngle(float a_angle);
bool GetTorsoPos(ConsoleRE::Actor* a_actor, ConsoleRE::NiPoint3& point);
bool GetTargetPointPosition(ConsoleRE::ObjectRefHandle a_target, const char* a_targetPoint, ConsoleRE::NiPoint3& a_outPos);

void SetRotationMatrix(ConsoleRE::NiMatrix3& a_matrix, float sacb, float cacb, float sb);
bool PredictAimProjectile(ConsoleRE::NiPoint3 a_projectilePos, ConsoleRE::NiPoint3 a_targetPosition, ConsoleRE::NiPoint3 a_targetVelocity, float a_gravity, ConsoleRE::NiPoint3& a_projectileVelocity);

[[nodiscard]] inline float GetPlayerTimeMultiplier()
{
	return GetPlayerTimeMult(*g_142EC5C60);
}

[[nodiscard]] inline float GetPlayerDeltaTime()
{
    return *g_deltaTime * GetPlayerTimeMultiplier();
}

[[nodiscard]] inline float GetRealTimeDeltaTime()
{
    return *g_deltaTimeRealTime;
}

[[nodiscard]] inline ConsoleRE::NiPoint3 TransformVectorByMatrix(const ConsoleRE::NiPoint3& a_vector, const ConsoleRE::NiMatrix3& a_matrix)
{
	return ConsoleRE::NiPoint3(a_matrix.entry[0][0] * a_vector.x + a_matrix.entry[0][1] * a_vector.y + a_matrix.entry[0][2] * a_vector.z,
		a_matrix.entry[1][0] * a_vector.x + a_matrix.entry[1][1] * a_vector.y + a_matrix.entry[1][2] * a_vector.z,
		a_matrix.entry[2][0] * a_vector.x + a_matrix.entry[2][1] * a_vector.y + a_matrix.entry[2][2] * a_vector.z);
}

[[nodiscard]] inline float AngleToRadian(float a_angle)
{
	return a_angle * 0.017453292f;
}

[[nodiscard]] inline float RadianToAngle(float a_radian)
{
	return a_radian * 57.295779513f;
}


[[nodiscard]] inline bool ApproximatelyEqual(float A, float B)
{
	return ((A - B) < FLT_EPSILON) && ((B - A) < FLT_EPSILON);
}

[[nodiscard]] inline ConsoleRE::NiPoint2 Vec2Rotate(const ConsoleRE::NiPoint2& vec, float angle)
{
	ConsoleRE::NiPoint2 ret;
	ret.x = vec.x * cos(angle) - vec.y * sin(angle);
	ret.y = vec.x * sin(angle) + vec.y * cos(angle);
	return ret;
}

[[nodiscard]] inline ConsoleRE::NiPoint3 RotateAngleAxis(const ConsoleRE::NiPoint3& vec, const float angle, const ConsoleRE::NiPoint3& axis)
{
	float S = sin(angle);
	float C = cos(angle);

	const float XX = axis.x * axis.x;
	const float YY = axis.y * axis.y;
	const float ZZ = axis.z * axis.z;

	const float XY = axis.x * axis.y;
	const float YZ = axis.y * axis.z;
	const float ZX = axis.z * axis.x;

	const float XS = axis.x * S;
	const float YS = axis.y * S;
	const float ZS = axis.z * S;

	const float OMC = 1.f - C;

	return ConsoleRE::NiPoint3(
		(OMC * XX + C) * vec.x + (OMC * XY - ZS) * vec.y + (OMC * ZX + YS) * vec.z,
		(OMC * XY + ZS) * vec.x + (OMC * YY + C) * vec.y + (OMC * YZ - XS) * vec.z,
		(OMC * ZX - YS) * vec.x + (OMC * YZ + XS) * vec.y + (OMC * ZZ + C) * vec.z
	);
}

[[nodiscard]] inline ConsoleRE::NiPoint3 RotateVector(const ConsoleRE::NiPoint3& a_vec, const ConsoleRE::NiQuaternion& a_quat)
{
	//http://people.csail.mit.edu/bkph/articles/Quaternions.pdf
	const ConsoleRE::NiPoint3 Q{ a_quat.x, a_quat.y, a_quat.z };
	const ConsoleRE::NiPoint3 T = Q.Cross(a_vec) * 2.f;
	return a_vec + (T * a_quat.w) + Q.Cross(T);
}

[[nodiscard]] inline ConsoleRE::NiPoint3 ClampSizeMax(const ConsoleRE::NiPoint3& vec, const float max)
{
	if (max < 1.e-4f)
	{
		return ConsoleRE::NiPoint3 {0, 0, 0};
	}

	const float squaredLength = vec.SqrLength();
	if (squaredLength > max * max) {
		const float scale = max * (1.0f / std::sqrt(squaredLength));
		return vec * scale;
	} else {
		return vec;
	}
}

//inline float ClampAngle(float angle, float min, float max)
//{
//	return fmod(angle, max - min) + min;
//}

[[nodiscard]] inline float ClipAngle(float angle, float min, float max)
{
	return fmin(max, fmax(min, angle));
}

[[nodiscard]] inline float GetAngle(ConsoleRE::NiPoint2& a, ConsoleRE::NiPoint2& b)
{
	return atan2(a.Cross(b), a.Dot(b));
}

[[nodiscard]] inline ConsoleRE::NiPoint3 ToOrientationRotation(const ConsoleRE::NiPoint3& a_vector)
{
	ConsoleRE::NiPoint3 ret;

	// Pitch
	ret.x = atan2(a_vector.z, std::sqrtf(a_vector.x * a_vector.x + a_vector.y * a_vector.y));

	// Roll
	ret.y = 0;

	// Yaw
	ret.z = atan2(a_vector.y, a_vector.x);

	return ret;
}

[[nodiscard]] inline ConsoleRE::NiPoint3 RotationToDirection(const float a_yaw, const float a_pitch)
{
	ConsoleRE::NiPoint3 ret;

	float CP, SP, CY, SY;
	CP = cos(a_pitch);
	SP = sin(a_pitch);
	CY = cos(a_yaw);
	SY = sin(a_yaw);

	ret.x = CP * CY;
	ret.y = CP * SY;
	ret.z = SP;

	return ret;
}

[[nodiscard]] inline ConsoleRE::NiPoint3 Project(const ConsoleRE::NiPoint3& A, const ConsoleRE::NiPoint3& B)
{
	return (B * ((A.x * B.x + A.y * B.y + A.z * B.z) / (B.x * B.x + B.y * B.y + B.z * B.z)));
}

[[nodiscard]] inline float Clamp(float value, float min, float max)
{
	return value < min ? min : value < max ? value : max;
}

[[nodiscard]] inline float InterpEaseIn(const float& A, const float& B, float alpha, float exp)
{
	float const modifiedAlpha = std::pow(alpha, exp);
	return std::lerp(A, B, modifiedAlpha);
}

[[nodiscard]] inline float InterpEaseOut(const float& A, const float& B, float alpha, float exp)
{
	float const modifiedAlpha = 1.f - pow(1.f - alpha, exp);
	return std::lerp(A, B, modifiedAlpha);
}

[[nodiscard]] inline float InterpEaseInOut(const float& A, const float& B, float alpha, float exp)
{
	return std::lerp(A, B, (alpha < 0.5f) ? InterpEaseIn(0.f, 1.f, alpha * 2.f, exp) * 0.5f : InterpEaseOut(0.f, 1.f, alpha * 2.f - 1.f, exp) * 0.5f + 0.5f);
}

[[nodiscard]] inline float InterpTo(float a_current, float a_target, float a_deltaTime, float a_interpSpeed)
{
	if (a_interpSpeed <= 0.f) {
		return a_target;
	}

	const float distance = a_target - a_current;

	if (distance * distance < FLT_EPSILON) {
		return a_target;
	}

	const float delta = distance * Clamp(a_deltaTime * a_interpSpeed, 0.f, 1.f);

	return a_current + delta;
}

[[nodiscard]] inline float InterpAngleTo(float a_current, float a_target, float a_deltaTime, float a_interpSpeed)
{
	if (a_interpSpeed <= 0.f) {
		return a_target;
	}

	const float distance = NormalRelativeAngle(a_target - a_current);

	if (distance * distance < FLT_EPSILON) {
		return a_target;
	}

	const float delta = distance * Clamp(a_deltaTime * a_interpSpeed, 0.f, 1.f);

	return a_current + delta;
}

[[nodiscard]] inline float GetAngleDiff(const float& A, const float& B)
{
	return PI - fabs(fmod(fabs(A - B), TWO_PI) - PI);
}

[[nodiscard]] inline bool FloatCompare(const float a, const float b)
{
	double delta = fabs(a - b);
	if (delta < std::numeric_limits<float>::epsilon() &&
		delta > -std::numeric_limits<float>::epsilon()) {
		return true;
	}
	return false;
}

[[nodiscard]] inline float GetPct(const float a_current, const float a_max)
{
	float percent = -1.f;

	if (a_max < 0.f) {
		return percent;
	}

	if (!FloatCompare(a_max, 0.f)) {
		//percent = ceil((a_current / a_max) * 100.f);
		percent = a_current / a_max;
		//return fmin(100.f, fmax(percent, -1.f));  // negative indicates that the actor value is not used
		return fmin(1.f, fmax(percent, -1.f));  // negative indicates that the actor value is not used
	}

	return percent;
}

[[nodiscard]] inline ConsoleRE::NiPoint3 GetNiPoint3(ConsoleRE::hkVector4 a_hkVector4)
{
	float quad[4];
	_mm_store_ps(quad, a_hkVector4.quad);
	return ConsoleRE::NiPoint3{ quad[0], quad[1], quad[2] };
}

[[nodiscard]] inline float Remap(const float a_oldValue, const float a_oldMin, const float a_oldMax, const float a_newMin, const float a_newMax)
{
	return (((a_oldValue - a_oldMin) * (a_newMax - a_newMin)) / (a_oldMax - a_oldMin)) + a_newMin;
}
