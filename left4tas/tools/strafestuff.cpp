// C++
// Strafe Stuff

/** This file is modifided version of HLTAS' hlstrafe used in SourcePauseTool
 * All credits go to the authors
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#pragma comment(lib, "mathlib.lib")

#include "../cvars.h"

#include "strafestuff.h"
#include "../utils/strafe_utils.h"

#include "mathlib/mathlib.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Strafe
{
	void MapSpeeds(ProcessedFrame& out, const MovementVars& vars)
	{
		if (out.Forward)
			out.ForwardSpeed += vars.Maxspeed;

		if (out.Back)
			out.ForwardSpeed -= vars.Maxspeed;

		if (out.Right)
			out.SideSpeed += vars.Maxspeed;

		if (out.Left)
			out.SideSpeed -= vars.Maxspeed;
	}

	double GetButtonsDirection(Button button)
	{
		switch (button)
		{
		case Button::FORWARD:
			return 0.0;
		case Button::FORWARD_LEFT:
			return M_PI / 4;
		case Button::LEFT:
			return M_PI / 2;
		case Button::BACK_LEFT:
			return 3 * M_PI / 4;
		case Button::BACK:
			return -M_PI;
		case Button::BACK_RIGHT:
			return -3 * M_PI / 4;
		case Button::RIGHT:
			return -M_PI / 2;
		case Button::FORWARD_RIGHT:
			return -M_PI / 4;
		default:
			return 0.0;
		}
	}

	Button GetBestButtons(double theta, bool right)
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

	double MaxAccelTheta(const PlayerData& player, const MovementVars& vars, bool onground, double wishspeed)
	{
		double accel = onground ? vars.Accelerate : vars.Airaccelerate;
		double accelspeed = accel * wishspeed * vars.EntFriction * vars.Frametime;

		if (accelspeed <= 0.0)
			return M_PI;

		if (player.Velocity.AsVector2D().IsZero(0))
			return 0.0;

		double wishspeed_capped = onground ? wishspeed : vars.WishspeedCap;
		double tmp = wishspeed_capped - accelspeed;

		if (tmp <= 0.0)
			return M_PI / 2;

		double speed = player.Velocity.Length2D();

		if (tmp < speed)
			return std::acos(tmp / speed);

		return 0.0;
	}

	double MaxAccelIntoYawTheta(const PlayerData& player, const MovementVars& vars, bool onground, double wishspeed, double vel_yaw, double yaw)
	{
		if (!player.Velocity.AsVector2D().IsZero(0))
			vel_yaw = Atan2(player.Velocity.y, player.Velocity.x);

		double theta = MaxAccelTheta(player, vars, onground, wishspeed);

		if (theta == 0.0 || theta == M_PI)
			return NormalizeRad(yaw - vel_yaw + theta);

		return std::copysign(theta, NormalizeRad(yaw - vel_yaw));
	}

	void SideStrafeGeneral(const PlayerData& player,
	                       const MovementVars& vars,
	                       bool onground,
	                       const StrafeButtons& strafeButtons,
	                       bool useGivenButtons,
	                       Button& usedButton,
	                       double vel_yaw,
	                       double theta,
	                       bool right,
	                       double& yaw)
	{
		if (useGivenButtons)
		{
			if (!onground)
			{
				if (right)
					usedButton = strafeButtons.AirRight;
				else
					usedButton = strafeButtons.AirLeft;
			}
			else
			{
				if (right)
					usedButton = strafeButtons.GroundRight;
				else
					usedButton = strafeButtons.GroundLeft;
			}
		}
		else
		{
			usedButton = GetBestButtons(theta, right);
		}

		double phi = GetButtonsDirection(usedButton);
		theta = right ? -theta : theta;

		if (!player.Velocity.AsVector2D().IsZero(0))
			vel_yaw = Atan2(player.Velocity.y, player.Velocity.x);

		yaw = NormalizeRad(vel_yaw - phi + theta);
	}

	double YawStrafeMaxAccel(PlayerData& player,
	                         const MovementVars& vars,
	                         bool onground,
	                         double wishspeed,
	                         const StrafeButtons& strafeButtons,
	                         bool useGivenButtons,
	                         Button& usedButton,
	                         double vel_yaw,
	                         double yaw)
	{
		double resulting_yaw;
		double theta = MaxAccelIntoYawTheta(player, vars, onground, wishspeed, vel_yaw, yaw);

		SideStrafeGeneral(player, vars, onground, strafeButtons, useGivenButtons, usedButton, vel_yaw, std::fabs(theta), (theta < 0), resulting_yaw);

		return resulting_yaw;
	}

	void StrafeVectorial(PlayerData& player, const MovementVars& vars, double target_yaw, double vel_yaw, ProcessedFrame& out, bool yawChanged)
	{
		ProcessedFrame dummy;

		// Get the desired strafe direction by calling the Strafe function while using forward strafe buttons
		Strafe(player, vars, target_yaw, vel_yaw, dummy, StrafeButtons(), true);

		// If forward is pressed, strafing should occur
		if (dummy.Forward)
		{
			// Outdated angle change stuff, now resides in aim.cpp
			if (!yawChanged && tas_strafe_vectorial_increment.GetFloat() > 0)
			{
				// Calculate updated yaw
				double adjustedTarget = NormalizeDeg(target_yaw + tas_strafe_vectorial_offset.GetFloat());
				double normalizedDiff = NormalizeDeg(adjustedTarget - vel_yaw);
				double additionAbs = std::min(static_cast<double>(tas_strafe_vectorial_increment.GetFloat()), std::abs(normalizedDiff));

				// Snap to target if difference too large (likely due to an ABH)
				if (std::abs(normalizedDiff) > tas_strafe_vectorial_snap.GetFloat())
					out.Yaw = adjustedTarget;
				else
					out.Yaw = vel_yaw + std::copysign(additionAbs, normalizedDiff);
			}
			else
			{
				out.Yaw = vel_yaw;
			}

			// Set move speeds to match the current yaw to produce the acceleration in direction thetaDeg
			double thetaDeg = dummy.Yaw;
			double diff = (out.Yaw - thetaDeg) * M_DEG2RAD;

			out.ForwardSpeed = static_cast<float>(std::cos(diff) * vars.Maxspeed);
			out.SideSpeed = static_cast<float>(std::sin(diff) * vars.Maxspeed);
			out.Processed = true;
		}
	}

	bool Strafe(PlayerData& player, const MovementVars& vars, double target_yaw, double vel_yaw, ProcessedFrame& out, const StrafeButtons& strafeButtons, bool useGivenButtons)
	{
		double wishspeed = vars.Maxspeed;

		if (vars.ReduceWishspeed)
			wishspeed *= 0.33333333f;

		Button usedButton = Button::FORWARD;

		out.Yaw = YawStrafeMaxAccel(player, vars, vars.OnGround, wishspeed, strafeButtons, useGivenButtons, usedButton, vel_yaw * M_DEG2RAD, target_yaw * M_DEG2RAD) * M_RAD2DEG;

		out.Forward = (usedButton == Button::FORWARD || usedButton == Button::FORWARD_LEFT || usedButton == Button::FORWARD_RIGHT);
		out.Back = (usedButton == Button::BACK || usedButton == Button::BACK_LEFT || usedButton == Button::BACK_RIGHT);
		out.Right = (usedButton == Button::RIGHT || usedButton == Button::FORWARD_RIGHT || usedButton == Button::BACK_RIGHT);
		out.Left = (usedButton == Button::LEFT || usedButton == Button::FORWARD_LEFT || usedButton == Button::BACK_LEFT);

		out.Processed = true;

		MapSpeeds(out, vars);

		return vars.OnGround;
	}

	void Friction(PlayerData& player, bool onground, const MovementVars& vars)
	{
		if (!onground)
			return;

		// Doing all this in floats, mismatch is too real otherwise.
		auto speed = player.Velocity.Length();
		if (speed < 0.1)
			return;

		auto friction = float{ vars.Friction * vars.EntFriction };
		auto control = (speed < vars.Stopspeed) ? vars.Stopspeed : speed;
		auto drop = control * friction * vars.Frametime;
		auto newspeed = std::max(speed - drop, 0.f);

		player.Velocity *= (newspeed / speed);
	}

	ConVar tas_strafe_autojump_min("tas_strafe_autojump_min", "200", FCVAR_RELEASE, "Min. velocity on ground to jump");
	ConVar tas_strafe_autojump_max("tas_strafe_autojump_max", "260", FCVAR_RELEASE, "Max. velocity on ground to jump");

	bool AutoJump(PlayerData& player, const MovementVars& vars)
	{
		double vel = player.Velocity.Length2D();

		if (vars.OnGround && vel <= tas_strafe_autojump_max.GetFloat() && vel >= tas_strafe_autojump_min.GetFloat())
			return true;

		return false;
	}
} // namespace Strafe

ConVar tas_strafe("tas_strafe", "0", FCVAR_RELEASE, "Enabled TAS strafing");

ConVar tas_strafe_autojump("tas_strafe_autojump", "0", FCVAR_RELEASE, "If enabled, jumps automatically when it's faster to move in the air than on ground.");

ConVar tas_strafe_yaw("tas_strafe_yaw", "", FCVAR_RELEASE, "Yaw to strafe to with tas_strafe_dir = 3.");

ConVar tas_strafe_buttons(
	"tas_strafe_buttons",
	"",
	FCVAR_RELEASE,
	"Sets the strafing buttons. The format is 4 digits together: \"<AirLeft><AirRight><GroundLeft><GroundRight>\". The default (auto-detect) is empty string: \"\".\nTable of buttons:\n\t0 - W\n\t1 - WA\n\t2 - A\n\t3 - SA\n\t4 - S\n\t5 - SD\n\t6 - D\n\t7 - WD");

ConVar tas_strafe_vectorial("tas_strafe_vectorial", "0", FCVAR_RELEASE, "Determines if strafing uses vectorial strafing");

ConVar tas_strafe_vectorial_increment(
	"tas_strafe_vectorial_increment",
	"2.5",
	FCVAR_RELEASE,
	"Determines how fast the player yaw angle moves towards the target yaw angle. 0 for no movement, 180 for instant snapping. Has no effect on strafing speed.");

ConVar tas_strafe_vectorial_offset("tas_strafe_vectorial_offset", "0", FCVAR_RELEASE, "Determines the target view angle offset from tas_strafe_yaw");

ConVar tas_strafe_vectorial_snap(
	"tas_strafe_vectorial_snap",
	"170",
	FCVAR_RELEASE,
	"Determines when the yaw angle snaps to the target yaw. Mainly used to prevent ABHing from resetting the yaw angle to the back on every jump.");

ConVar tas_strafe_ignore_ground("tas_strafe_ignore_ground", "0", FCVAR_RELEASE, "Strafe only when in air.");

ConVar tas_strafe_tickrate("tas_strafe_tickrate", "", FCVAR_RELEASE, "Sets the value of tickrate used in strafing. If empty, uses the default value.");

ConVar tas_force_wishspeed_cap("tas_force_wishspeed_cap", "", FCVAR_RELEASE, "Sets the value of the wishspeed cap used in TAS calculations. If empty, uses the default value: 30.");