/** This file is modifided version of hlstrafe used in SourcePauseTool
 * Original authors are YaLTeR, Matherunner, Jukspa
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#pragma once

namespace Strafe
{
	enum class StrafeType : unsigned char
	{
		MAXACCEL = 0,
		MAXANGLE,
		MAXDECCEL,
		CONSTSPEED
	};

	enum class StrafeDir : unsigned char
	{
		LEFT = 0,
		RIGHT,
		BEST,
		YAW,
		POINT
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
		StrafeButtons() : AirLeft(Button::FORWARD), AirRight(Button::FORWARD), GroundLeft(Button::FORWARD), GroundRight(Button::FORWARD)
		{
		}

		Button AirLeft;
		Button AirRight;
		Button GroundLeft;
		Button GroundRight;
	};

	struct PlayerData
	{
		PlayerData() : Origin{ 0.0f, 0.0f, 0.0f }, Velocity{ 0.0f, 0.0f, 0.0f }, SpeedBeforeFriction(0.0)
		{
		}

		float Origin[3];
		float Velocity[3];

		double SpeedBeforeFriction;
	};

	struct MovementVars
	{
		float Frametime = 1.0f / 30.0f;
		float Maxspeed;
		float Stopspeed;
		float Friction;
		float EntFriction;
		float Accelerate;
		float Airaccelerate;

		bool OnGround;
		bool ReduceWishspeed;
	};

	struct ProcessedFrame
	{
		ProcessedFrame() : Yaw(0.0f), Forward(false), Left(false), Right(false), Back(false), Processed(false), Forwardspeed(0.0f), Sidespeed(0.0f)
		{
		}

		float Yaw;

		bool Forward;
		bool Left;
		bool Right;
		bool Back;

		bool Processed;

		float Forwardspeed;
		float Sidespeed;
	};

	struct Frame
	{
		Frame() :
			Strafe(false),
			StrafeVectorial(false),
			StrafeToViewAngles(false),
			UseGivenButtons(false),
			IgnoreGround(false),
			VectorialIncrement(2.5f),
			VectorialOffset(0.0f),
			VectorialSnap(170.0f),
			Type(StrafeType::MAXACCEL),
			Dir(StrafeDir::YAW),
			Yaw(0.0),
			X(0.0),
			Y(0.0)
		{
		}

	public:
		float VectorialIncrement;
		float VectorialOffset;
		float VectorialSnap;

		StrafeButtons buttons;
		StrafeType Type;
		StrafeDir Dir;

		bool Strafe;
		bool StrafeVectorial;
		bool StrafeToViewAngles;
		bool UseGivenButtons;
		bool IgnoreGround;

		double Yaw;
		double X;
		double Y;

	public:
		inline StrafeType GetType() const { return Type; }
		inline StrafeDir GetDir() const { return Dir; }
		inline void SetType(StrafeType value) { Type = value; }
		inline void SetDir(StrafeDir value) { Dir = value; }

		inline double GetYaw() const { return Yaw; }
		inline double GetX() const { return X; }
		inline double GetY() const { return Y; }

		inline void SetYaw(double value) { Yaw = value; }
		inline void SetX(double value) { X = value; }
		inline void SetY(double value) { Y = value; }
	};

	struct StrafeData
	{
		PlayerData player;
		MovementVars vars;
		Frame frame;
	};

	/*
		Computes the new velocity given unit acceleration vector and wishspeed
		and stores the result in player.Velocity.
	*/
	void VectorFME(PlayerData &player, MovementVars &vars, double wishspeed, const double a[2]);

	/*
		Applies the ground friction the same way as PM_Friction would, changing player.Velocity.
	*/
	void Friction(StrafeData &strafedata);

	/*
		Vectorial strafing function
	*/
	void StrafeVectorial(StrafeData &strafedata, ProcessedFrame& out, bool yawChanged);

	/*
		Main strafe function
	*/
	void Strafe(StrafeData &strafedata, ProcessedFrame& out);

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal speed gain.
	*/
	double MaxAccelTheta(StrafeData &strafedata, double wishspeed);

	/*
		Returns the angle between velocity and wishdir in [0, Pi] that will
		keep the speed constant as far as possible. Under certain conditions
		the angle from MaxAccelTheta will be returned.
	*/
	double ConstSpeedTheta(StrafeData &strafedata, double wishspeed);

	/*
		Returns the angle in radians - [-Pi; Pi) - between velocity and wishdir that will
		result in maximal speed gain into the given yaw - [-Pi; Pi). If velocity is zero, vel_yaw will
		be used in place of velocity angle.
	*/
	double MaxAccelIntoYawTheta(StrafeData &strafedata, double wishspeed, double vel_yaw, double yaw);

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal velocity angle change.
	*/
	double MaxAngleTheta(StrafeData &strafedata, double wishspeed);

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal decceleration.
	*/
	double MaxDeccelTheta(StrafeData &strafedata, double wishspeed);

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes sideways with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double SideStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right);
	double SideStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right);
	double SideStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right, bool& strafed);
	double SideStrafeConstSpeed(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool right);

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes to the best dir with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double BestStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw);
	double BestStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw);
	double BestStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, bool& strafed);
	double BestStrafeConstSpeed(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw);

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes to the given yaw with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double YawStrafeMaxAccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw);
	double YawStrafeMaxAngle(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw);
	double YawStrafeMaxDeccel(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw, bool &strafed);
	double YawStrafeConstSpeed(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, double yaw);

	/*
		Finds the best yaw to use for the given strafe type taking the anglemod compensation into account, then
		strafes to the given point if needed with that yaw and returns it in radians, given fixed buttons. If not strafing
		is better, set strafed to false, otherwise - to true.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double PointStrafe(StrafeData &strafedata, double wishspeed, Button& usedButton, double vel_yaw, StrafeType type, double point[2], bool& strafed);
}