/** This file is modifided version of HLTAS' hlstrafe used in SourcePauseTool
 * Original authors are YaLTeR, Matherunner, Jukspa
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#include "strafe.h"
#include "strafe_utils.h"

#include <algorithm>

namespace Strafe
{
	void VectorFME(PlayerData &player, MovementVars &vars, double wishspeed, const double a[2])
	{
		double wishspeed_capped = vars.OnGround ? wishspeed : 30;
		double tmp = wishspeed_capped - DotProduct<float, double, 2>(player.Velocity, a);

		if (tmp <= 0.0)
			return;

		double accel = vars.OnGround ? vars.Accelerate : vars.Airaccelerate;
		double accelspeed = accel * wishspeed * vars.EntFriction * vars.Frametime;

		if (accelspeed <= tmp)
			tmp = accelspeed;

		player.Velocity[0] += static_cast<float>(a[0] * tmp);
		player.Velocity[1] += static_cast<float>(a[1] * tmp);
	}

	static inline void MapSpeeds(ProcessedFrame& out, const MovementVars& vars)
	{
		if (out.Forward)
			out.Forwardspeed += vars.Maxspeed;

		if (out.Back)
			out.Forwardspeed -= vars.Maxspeed;

		if (out.Right)
			out.Sidespeed += vars.Maxspeed;

		if (out.Left)
			out.Sidespeed -= vars.Maxspeed;
	}	
	
	static inline double ButtonsPhi(Button button)
	{
		switch (button)
		{
		case Button::FORWARD: return 0;
		case Button::FORWARD_LEFT: return M_PI / 4;
		case Button::LEFT: return M_PI / 2;
		case Button::BACK_LEFT: return 3 * M_PI / 4;
		case Button::BACK: return -M_PI;
		case Button::BACK_RIGHT: return -3 * M_PI / 4;
		case Button::RIGHT: return -M_PI / 2;
		case Button::FORWARD_RIGHT: return -M_PI / 4;
		default: return 0;
		}
	}

	static inline Button GetBestButtons(double theta, bool right)
	{
		if (theta < M_PI / 8)
			return Button::FORWARD;
		else if (theta < 3 * M_PI / 8)
			return right ? Button::FORWARD_RIGHT : Button::FORWARD_LEFT;
		else if (theta < 5 * M_PI / 8)
			return right ? Button::RIGHT : Button::LEFT;
		else if (theta < 7 * M_PI / 8)
			return right ? Button::BACK_RIGHT : Button::BACK_LEFT;
		else
			return Button::BACK;
	}

	double MaxAccelTheta(StrafeData &strafedata, double wishspeed)
	{
		double accel = strafedata.vars.OnGround ? strafedata.vars.Accelerate : strafedata.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * strafedata.vars.EntFriction * strafedata.vars.Frametime;

		if (accelspeed <= 0.0)
			return M_PI;

		if (IsZero<float, 2>(strafedata.player.Velocity))
			return 0.0;

		double wishspeed_capped = strafedata.vars.OnGround ? wishspeed : 30;
		double tmp = wishspeed_capped - accelspeed;

		if (tmp <= 0.0)
			return M_PI / 2;

		double speed = Length<float, 2>(strafedata.player.Velocity);

		if (tmp < speed)
			return std::acos(tmp / speed);

		return 0.0;
	}

	double ConstSpeedTheta(StrafeData &strafedata, double wishspeed)
	{
		double gamma1 = strafedata.vars.EntFriction * strafedata.vars.Frametime * wishspeed;
		double speedsqr = DotProduct<float, float, 2>(strafedata.player.Velocity, strafedata.player.Velocity);
		double numer, denom;

		if (strafedata.vars.OnGround)
		{
			gamma1 *= strafedata.vars.Accelerate;

			double sqrdiff = strafedata.player.SpeedBeforeFriction * strafedata.player.SpeedBeforeFriction - speedsqr;
			double tmp = sqrdiff / gamma1;

			if (tmp + gamma1 <= 2 * wishspeed)
			{
				numer = tmp - gamma1;
				denom = 2 * std::sqrt(speedsqr);
			}
			else if (gamma1 > wishspeed && wishspeed * wishspeed >= sqrdiff)
			{
				numer = -std::sqrt(wishspeed * wishspeed - sqrdiff);
				denom = std::sqrt(speedsqr);
			}
			else
			{
				return MaxAccelTheta(strafedata, wishspeed);
			}
		}
		else
		{
			gamma1 *= strafedata.vars.Airaccelerate;

			if (gamma1 <= 60)
			{
				numer = -gamma1;
				denom = 2 * std::sqrt(speedsqr);
			}
			else
			{
				numer = -30;
				denom = std::sqrt(speedsqr);
			}
		}

		if (denom < std::fabs(numer))
			return MaxAccelTheta(strafedata, wishspeed);

		return std::acos(numer / denom);
	}

	double MaxAccelIntoYawTheta(StrafeData &strafedata, double wishspeed, double vel_yaw, double yaw)
	{
		if (!IsZero<float, 2>(strafedata.player.Velocity))
			vel_yaw = Atan2(strafedata.player.Velocity[1], strafedata.player.Velocity[0]);

		double theta = MaxAccelTheta(strafedata, wishspeed);

		if (theta == 0.0 || theta == M_PI)
			return NormalizeRad(yaw - vel_yaw + theta);

		return std::copysign(theta, NormalizeRad(yaw - vel_yaw));
	}

	double MaxAngleTheta(StrafeData &strafedata, double wishspeed)
	{
		double speed = Length<float, 2>(strafedata.player.Velocity);
		double accel = strafedata.vars.OnGround ? strafedata.vars.Accelerate : strafedata.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * strafedata.vars.EntFriction * strafedata.vars.Frametime;

		if (accelspeed <= 0.0)
		{
			double wishspeed_capped = strafedata.vars.OnGround ? wishspeed : 30;
			accelspeed *= -1;

			if (accelspeed >= speed)
			{
				if (wishspeed_capped >= speed)
					return 0.0;
				else
					return std::acos(wishspeed_capped / speed); // The actual angle needs to be _less_ than this.
			}
			else
			{
				if (wishspeed_capped >= speed)
					return std::acos(accelspeed / speed);
				else
					return std::acos(std::min(accelspeed, wishspeed_capped) / speed); // The actual angle needs to be _less_ than this if wishspeed_capped <= accelspeed.
			}
		}
		else
		{
			if (accelspeed >= speed)
				return M_PI;
			else
				return std::acos(-1 * accelspeed / speed);
		}
	}

	double MaxDeccelTheta(StrafeData &strafedata, double wishspeed)
	{
		double speed = Length<float, 2>(strafedata.player.Velocity);
		double accel = strafedata.vars.OnGround ? strafedata.vars.Accelerate : strafedata.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * strafedata.vars.EntFriction * strafedata.vars.Frametime;

		if (accelspeed < 0.0)
		{
			double wishspeed_capped = strafedata.vars.OnGround ? wishspeed : 30;
			return std::acos(wishspeed_capped / speed); // The actual angle needs to be _less_ than this.
		}
		else
		{
			return M_PI;
		}
	}

	static double SideStrafeGeneral(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double theta, bool right, float new_vel[2])
	{
		if (!IsZero<float, 2>(strafedata.player.Velocity))
			vel_yaw = Atan2(strafedata.player.Velocity[1], strafedata.player.Velocity[0]);

		if (strafedata.frame.UseGivenButtons)
		{
			if (strafedata.vars.OnGround)
			{
				if (right)
					usedButton = strafedata.frame.buttons.GroundRight;
				else
					usedButton = strafedata.frame.buttons.GroundLeft;
			}
			else
			{
				if (right)
					usedButton = strafedata.frame.buttons.AirRight;
				else
					usedButton = strafedata.frame.buttons.AirLeft;
			}
		}
		else
		{
			// If the velocity is zero, theta is based on the viewangle yaw, which means the button
			// will be based on the viewangle yaw, which is wrong. Force the button to forward when
			// the velocity is zero, this makes sense as it will be set to forward anyway as soon
			// as the velocity becomes non-zero (because theta is small initially).

			if (IsZero<float, 2>(strafedata.player.Velocity))
				usedButton = Button::FORWARD;
			else
				usedButton = GetBestButtons(theta, right);
		}

		double phi = ButtonsPhi(usedButton);
		theta = right ? -theta : theta;

		double yaw = NormalizeRad(vel_yaw - phi + theta);

		double avec[2] = { std::cos(yaw + phi), std::sin(yaw + phi) };

		PlayerData pl = strafedata.player;
		VectorFME(pl, strafedata.vars, wishspeed, avec);
		VecCopy<float, 2>(pl.Velocity, new_vel);

		return yaw;
	}

	double SideStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right)
	{
		double theta = MaxAccelTheta(strafedata, wishspeed);

		float new_vel[2];

		double yaw = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, right, new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return yaw;
	}

	double SideStrafeConstSpeed(StrafeData& strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right)
	{
		double theta = ConstSpeedTheta(strafedata, wishspeed);

		float new_vel[2];

		double yaw = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, right, new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return yaw;
	}

	double SideStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right)
	{
		double theta = MaxAngleTheta(strafedata, wishspeed);

		float new_vel[2];

		double yaw = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, right, new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return yaw;
	}

	double SideStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right, bool& strafed)
	{
		// Check a bunch of stuff.
		double speed = Length<float, 2>(strafedata.player.Velocity);
		double accel = strafedata.vars.OnGround ? strafedata.vars.Accelerate : strafedata.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * strafedata.vars.EntFriction * strafedata.vars.Frametime;
		double wishspeed_capped = strafedata.vars.OnGround ? wishspeed : 30;

		if (speed == 0.0 || accelspeed == 0.0 || (accelspeed < 0.0 && accelspeed <= -wishspeed_capped * 2))
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double theta = MaxDeccelTheta(strafedata, wishspeed);

		float new_vel[2];

		double yaw = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, right, new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return yaw;
	}

	double BestStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw)
	{
		float temp_vel[2], orig_vel[2];
		double yaws[2];

		Button buttons[2];

		VecCopy<float, 2>(strafedata.player.Velocity, orig_vel);

		yaws[0] = SideStrafeMaxAccel(strafedata, wishspeed, buttons[0], vel_yaw, false);

		VecCopy<float, 2>(strafedata.player.Velocity, temp_vel);
		VecCopy<float, 2>(orig_vel, strafedata.player.Velocity);

		yaws[1] = SideStrafeMaxAccel(strafedata, wishspeed, buttons[1], vel_yaw, true);

		double speedsqrs[2] =
		{
			DotProduct<float, float, 2>(temp_vel, temp_vel),
			DotProduct<float, float, 2>(strafedata.player.Velocity, strafedata.player.Velocity)
		};

		if (speedsqrs[0] > speedsqrs[1])
		{
			VecCopy<float, 2>(temp_vel, strafedata.player.Velocity);
			usedButton = buttons[0];
			return yaws[0];
		}
		else
		{
			usedButton = buttons[1];
			return yaws[1];
		}
	}

	double BestStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw)
	{
		float temp_vel[2], orig_vel[2];
		double yaws[2];

		Button buttons[2];

		VecCopy<float, 2>(strafedata.player.Velocity, orig_vel);

		yaws[0] = SideStrafeMaxAngle(strafedata, wishspeed, buttons[0], vel_yaw, false);

		VecCopy<float, 2>(strafedata.player.Velocity, temp_vel);
		VecCopy<float, 2>(orig_vel, strafedata.player.Velocity);

		yaws[1] = SideStrafeMaxAngle(strafedata, wishspeed, buttons[1], vel_yaw, true);

		double old_speed = Length<float, 2>(orig_vel);
		double speeds[2] = { Length<float, 2>(temp_vel), Length<float, 2>(strafedata.player.Velocity) };

		double cosangles[2] =
		{
			DotProduct<float, float, 2>(temp_vel, orig_vel) / (old_speed * speeds[0]),
			DotProduct<float, float, 2>(strafedata.player.Velocity, orig_vel) / (old_speed * speeds[1])
		};

		if (cosangles[0] < cosangles[1])
		{
			VecCopy<float, 2>(temp_vel, strafedata.player.Velocity);
			usedButton = buttons[0];
			return yaws[0];
		}
		else
		{
			usedButton = buttons[1];
			return yaws[1];
		}
	}

	double BestStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool& strafed)
	{
		float temp_vel[2], orig_vel[2];
		double yaws[2];

		Button buttons[2];

		VecCopy<float, 2>(strafedata.player.Velocity, orig_vel);

		yaws[0] = SideStrafeMaxDeccel(strafedata, wishspeed, buttons[0], vel_yaw, false, strafed);

		VecCopy<float, 2>(strafedata.player.Velocity, temp_vel);
		VecCopy<float, 2>(orig_vel, strafedata.player.Velocity);

		yaws[1] = SideStrafeMaxDeccel(strafedata, wishspeed, buttons[1], vel_yaw, true, strafed);

		// The condition for strafed does not depend on the strafing direction so
		// either both functions returned true, or both returned false.
		if (!strafed)
			return 0.0;

		double speedsqrs[2] =
		{
			DotProduct<float, float, 2>(temp_vel, temp_vel),
			DotProduct<float, float, 2>(strafedata.player.Velocity, strafedata.player.Velocity)
		};

		if (speedsqrs[0] < speedsqrs[1])
		{
			VecCopy<float, 2>(temp_vel, strafedata.player.Velocity);
			usedButton = buttons[0];
			return yaws[0];
		}
		else
		{
			usedButton = buttons[1];
			return yaws[1];
		}
	}

	double BestStrafeConstSpeed(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw)
	{
		float temp_vel[2], orig_vel[2];
		double yaws[2];

		Button buttons[2];

		VecCopy<float, 2>(strafedata.player.Velocity, orig_vel);

		yaws[0] = SideStrafeConstSpeed(strafedata, wishspeed, buttons[0], vel_yaw, false);

		VecCopy<float, 2>(strafedata.player.Velocity, temp_vel);
		VecCopy<float, 2>(orig_vel, strafedata.player.Velocity);

		yaws[1] = SideStrafeConstSpeed(strafedata, wishspeed, buttons[1], vel_yaw, true);

		double speedsqrs[2] =
		{
			DotProduct<float, float, 2>(temp_vel, temp_vel),
			DotProduct<float, float, 2>(strafedata.player.Velocity, strafedata.player.Velocity)
		};

		double oldspeedsqr = DotProduct<float, float, 2>(orig_vel, orig_vel);

		if (std::fabs(oldspeedsqr - speedsqrs[0]) <= std::fabs(oldspeedsqr - speedsqrs[1]))
		{
			VecCopy<float, 2>(temp_vel, strafedata.player.Velocity);
			usedButton = buttons[0];
			return yaws[0];
		}
		else
		{
			usedButton = buttons[1];
			return yaws[1];
		}
	}

	double YawStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw)
	{
		double theta = MaxAccelIntoYawTheta(strafedata, wishspeed, vel_yaw, yaw);

		float new_vel[2];

		double result = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, std::fabs(theta), (theta < 0), new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return result;
	}

	double YawStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw)
	{
		double theta = MaxAngleTheta(strafedata, wishspeed);

		float new_vel[2];

		if (!IsZero<float, 2>(strafedata.player.Velocity))
			vel_yaw = Atan2(strafedata.player.Velocity[1], strafedata.player.Velocity[0]);
		
		double result = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, (NormalizeRad(yaw - vel_yaw) < 0), new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return result;
	}

	double YawStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw, bool& strafed)
	{
		// Check a bunch of stuff.
		double speed = Length<float, 2>(strafedata.player.Velocity);
		double accel = strafedata.vars.OnGround ? strafedata.vars.Accelerate : strafedata.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * strafedata.vars.EntFriction * strafedata.vars.Frametime;
		double wishspeed_capped = strafedata.vars.OnGround ? wishspeed : 30;

		if (speed == 0.0 || accelspeed == 0.0 || (accelspeed < 0.0 && accelspeed <= -wishspeed_capped * 2))
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double theta = MaxDeccelTheta(strafedata, wishspeed);

		float new_vel[2];

		vel_yaw = Atan2(strafedata.player.Velocity[1], strafedata.player.Velocity[0]);
		double result = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, (NormalizeRad(yaw - vel_yaw) < 0), new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return result;
	}

	double YawStrafeConstSpeed(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw)
	{
		double theta = ConstSpeedTheta(strafedata, wishspeed);

		float new_vel[2];

		double result = SideStrafeGeneral(strafedata, wishspeed, usedButton, vel_yaw, theta, (NormalizeRad(yaw - vel_yaw) < 0), new_vel);

		VecCopy<float, 2>(new_vel, strafedata.player.Velocity);
		return result;
	}

	double PointStrafe(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, StrafeType type, double point[2], bool& strafed)
	{
		// Covers the case where both vectors are zero.
		if (Distance<float, double, 2>(strafedata.player.Origin, point) <= 2.0)
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double difference[2];
		VecSubtract<double, float, 2>(point, strafedata.player.Origin, difference);
		double yaw = Atan2(difference[1], difference[0]);

		strafedata.frame.SetYaw(yaw * M_RAD2DEG);

		switch (type)
		{
		default:
		case StrafeType::MAXACCEL: return YawStrafeMaxAccel(strafedata, wishspeed, usedButton, vel_yaw, yaw);
		case StrafeType::MAXANGLE: return YawStrafeMaxAngle(strafedata, wishspeed, usedButton, vel_yaw, yaw);
		case StrafeType::MAXDECCEL: return YawStrafeMaxDeccel(strafedata, wishspeed, usedButton, vel_yaw, yaw, strafed);
		case StrafeType::CONSTSPEED: return YawStrafeConstSpeed(strafedata, wishspeed, usedButton, vel_yaw, yaw);
		// TODO add the rest of the calls when the functions are done.
		}
	}

	void StrafeVectorial(StrafeData &strafedata, ProcessedFrame& out, bool yawChanged)
	{
		ProcessedFrame dummy;

		bool useGivenButtons = strafedata.frame.UseGivenButtons;
		StrafeButtons buttons = strafedata.frame.buttons;

		strafedata.frame.UseGivenButtons = true;
		strafedata.frame.buttons = StrafeButtons();

		Strafe(strafedata, dummy);

		if (dummy.Forward)
		{
			if (!yawChanged && strafedata.frame.VectorialIncrement > 0)
			{
				double adjustedTarget = NormalizeDeg(strafedata.frame.GetYaw() + strafedata.frame.VectorialOffset);
				double normalizedDiff = NormalizeDeg(adjustedTarget - out.Yaw);
				double additionAbs = std::min(static_cast<double>(strafedata.frame.VectorialIncrement), std::abs(normalizedDiff));

				if (std::abs(normalizedDiff) > strafedata.frame.VectorialSnap)
					out.Yaw = adjustedTarget;
				else
					out.Yaw = out.Yaw + std::copysign(additionAbs, normalizedDiff);
			}

			double thetaDeg = dummy.Yaw;
			double diff = (out.Yaw - thetaDeg) * M_DEG2RAD;

			out.Forwardspeed = static_cast<float>(std::cos(diff) * strafedata.vars.Maxspeed);
			out.Sidespeed = static_cast<float>(std::sin(diff) * strafedata.vars.Maxspeed);
			out.Processed = true;
		}

		strafedata.frame.UseGivenButtons = useGivenButtons;
		strafedata.frame.buttons = buttons;
	}

	void Strafe(StrafeData &strafedata, ProcessedFrame& out)
	{
		double wishspeed = strafedata.vars.Maxspeed;

		if (strafedata.vars.ReduceWishspeed)
			wishspeed *= 1.0 / 3.0;

		bool strafed = true;
		Button usedButton;

		//double vel_yaw = 0.0;
		double vel_yaw = out.Yaw * M_DEG2RAD;

		switch (strafedata.frame.GetDir())
		{
		case StrafeDir::LEFT:
			if (strafedata.frame.GetType() == StrafeType::MAXACCEL)
				out.Yaw = static_cast<float>(SideStrafeMaxAccel(strafedata, wishspeed, usedButton, vel_yaw, false) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXANGLE)
				out.Yaw = static_cast<float>(SideStrafeMaxAngle(strafedata, wishspeed, usedButton, vel_yaw, false) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::CONSTSPEED)
				out.Yaw = static_cast<float>(SideStrafeConstSpeed(strafedata, wishspeed, usedButton, vel_yaw, false) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXDECCEL)
			{
				auto yaw = static_cast<float>(SideStrafeMaxDeccel(strafedata, wishspeed, usedButton, vel_yaw, false, strafed) * M_RAD2DEG);

				if (strafed)
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::RIGHT:
			if (strafedata.frame.GetType() == StrafeType::MAXACCEL)
				out.Yaw = static_cast<float>(SideStrafeMaxAccel(strafedata, wishspeed, usedButton, vel_yaw, true) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXANGLE)
				out.Yaw = static_cast<float>(SideStrafeMaxAngle(strafedata, wishspeed, usedButton, vel_yaw, true) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::CONSTSPEED)
				out.Yaw = static_cast<float>(SideStrafeConstSpeed(strafedata, wishspeed, usedButton, vel_yaw, true) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXDECCEL)
			{
				auto yaw = static_cast<float>(SideStrafeMaxDeccel(strafedata, wishspeed, usedButton, vel_yaw, true, strafed) * M_RAD2DEG);

				if (strafed)
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::BEST:
			if (strafedata.frame.GetType() == StrafeType::MAXACCEL)
				out.Yaw = static_cast<float>(BestStrafeMaxAccel(strafedata, wishspeed, usedButton, vel_yaw) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXANGLE)
				out.Yaw = static_cast<float>(BestStrafeMaxAngle(strafedata, wishspeed, usedButton, vel_yaw) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::CONSTSPEED)
				out.Yaw = static_cast<float>(BestStrafeConstSpeed(strafedata, wishspeed, usedButton, vel_yaw) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXDECCEL)
			{
				auto yaw = static_cast<float>(BestStrafeMaxDeccel(strafedata, wishspeed, usedButton, vel_yaw, strafed) * M_RAD2DEG);

				if (strafed)
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::YAW:
			if (strafedata.frame.GetType() == StrafeType::MAXACCEL)
				out.Yaw = static_cast<float>(YawStrafeMaxAccel(strafedata, wishspeed, usedButton, vel_yaw, strafedata.frame.GetYaw() * M_DEG2RAD) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXANGLE)
				out.Yaw = static_cast<float>(YawStrafeMaxAngle(strafedata, wishspeed, usedButton, vel_yaw, strafedata.frame.GetYaw() * M_DEG2RAD) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::CONSTSPEED)
				out.Yaw = static_cast<float>(YawStrafeConstSpeed(strafedata, wishspeed, usedButton, vel_yaw, strafedata.frame.GetYaw() * M_DEG2RAD) * M_RAD2DEG);
			else if (strafedata.frame.GetType() == StrafeType::MAXDECCEL)
			{
				auto yaw = static_cast<float>(YawStrafeMaxDeccel(strafedata, wishspeed, usedButton, vel_yaw, strafedata.frame.GetYaw() * M_DEG2RAD, strafed) * M_RAD2DEG);

				if (strafed)
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::POINT:
		{
			double point[] = { strafedata.frame.GetX(), strafedata.frame.GetY() };
			auto yaw = static_cast<float>(PointStrafe(strafedata, wishspeed, usedButton, vel_yaw, strafedata.frame.GetType(), point, strafed) * M_RAD2DEG);

			if (strafed)
				out.Yaw = yaw;
		}
			break;

		default:
			strafed = false;
			break;
		}

		if (strafed)
		{
			out.Forward = (usedButton == Button::FORWARD || usedButton == Button::FORWARD_LEFT || usedButton == Button::FORWARD_RIGHT);
			out.Back = (usedButton == Button::BACK || usedButton == Button::BACK_LEFT || usedButton == Button::BACK_RIGHT);
			out.Right = (usedButton == Button::RIGHT || usedButton == Button::FORWARD_RIGHT || usedButton == Button::BACK_RIGHT);
			out.Left = (usedButton == Button::LEFT || usedButton == Button::FORWARD_LEFT || usedButton == Button::BACK_LEFT);
			out.Processed = true;

			MapSpeeds(out, strafedata.vars);
		}
	}
	
	void Friction(StrafeData &strafedata)
	{
		strafedata.player.SpeedBeforeFriction = Length<float, 2>(strafedata.player.Velocity);

		if (!strafedata.vars.OnGround)
			return;

		// Doing all this in floats, mismatch is too real otherwise.
		auto speed = static_cast<float>( std::sqrt(static_cast<double>(strafedata.player.Velocity[0] * strafedata.player.Velocity[0] +
																	   strafedata.player.Velocity[1] * strafedata.player.Velocity[1] +
																	   strafedata.player.Velocity[2] * strafedata.player.Velocity[2])) );
		if (speed < 0.1)
			return;

		auto friction = float{ strafedata.vars.Friction * strafedata.vars.EntFriction };
		auto control = (speed < strafedata.vars.Stopspeed) ? strafedata.vars.Stopspeed : speed;
		auto drop = control * friction * strafedata.vars.Frametime;
		auto newspeed = std::max(speed - drop, 0.f);

		VecScale<float, 3>(strafedata.player.Velocity, newspeed / speed, strafedata.player.Velocity);
	}
}