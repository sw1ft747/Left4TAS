// C++
// Strafe Stuff

/** This file is modifided version of HLTAS' hlstrafe used in SourcePauseTool
 * All credits go to the authors
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#pragma once

namespace Strafe
{
	struct MovementVars
	{
		float Accelerate;
		float Airaccelerate;
		float EntFriction;
		float Frametime;
		float Friction;
		float Maxspeed;
		float Stopspeed;
		float WishspeedCap;

		bool OnGround;
		bool ReduceWishspeed;
	};

	struct PlayerData
	{
		Vector Velocity;
	};

	enum class Button : unsigned char
	{
		FORWARD = 0,
		FORWARD_LEFT,
		LEFT,
		BACK_LEFT,
		BACK,
		BACK_RIGHT,
		RIGHT,
		FORWARD_RIGHT
	};

	struct StrafeButtons
	{
		StrafeButtons()
		    : AirLeft(Button::FORWARD)
		    , AirRight(Button::FORWARD)
		    , GroundLeft(Button::FORWARD)
		    , GroundRight(Button::FORWARD)
		{
		}

		Button AirLeft;
		Button AirRight;
		Button GroundLeft;
		Button GroundRight;
	};

	struct ProcessedFrame
	{
		bool Processed;
		bool Forward;
		bool Back;
		bool Right;
		bool Left;

		double Yaw;
		float ForwardSpeed;
		float SideSpeed;

		ProcessedFrame()
		    : Processed(false)
		    , Forward(false)
		    , Back(false)
		    , Right(false)
		    , Left(false)
		    , Yaw(0)
		    , ForwardSpeed(0)
		    , SideSpeed(0)
		{
		}
	};

	// Convert both arguments to doubles.
	double Atan2(double a, double b);

	void MapSpeeds(ProcessedFrame& out, const MovementVars& vars);

	double GetButtonsDirection(Button button);

	Button GetBestButtons(double theta, bool right);

	double MaxAccelTheta(const PlayerData& player, const MovementVars& vars, bool onground, double wishspeed);

	double MaxAccelIntoYawTheta(const PlayerData& player, const MovementVars& vars, bool onground, double wishspeed, double vel_yaw, double yaw);

	void SideStrafeGeneral(const PlayerData& player,
	                       const MovementVars& vars,
	                       bool onground,
	                       const StrafeButtons& strafeButtons,
	                       bool useGivenButtons,
	                       Button& usedButton,
	                       double vel_yaw,
	                       double theta,
	                       bool right,
	                       double& yaw);

	double YawStrafeMaxAccel(PlayerData& player,
	                         const MovementVars& vars,
	                         bool onground,
	                         double wishspeed,
	                         const StrafeButtons& strafeButtons,
	                         bool useGivenButtons,
	                         Button& usedButton,
	                         double vel_yaw,
	                         double yaw);

	void StrafeVectorial(PlayerData& player, const MovementVars& vars, double target_yaw, double vel_yaw, ProcessedFrame& out, bool lockCamera);

	bool Strafe(PlayerData& player, const MovementVars& vars, double target_yaw, double vel_yaw, ProcessedFrame& out, const StrafeButtons& strafeButtons, bool useGivenButtons);

	void Friction(PlayerData& player, bool onground, const MovementVars& vars);

	bool AutoJump(PlayerData& player, const MovementVars& vars);
} // namespace Strafe