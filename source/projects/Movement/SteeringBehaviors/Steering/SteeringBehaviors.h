/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../SteeringHelpers.h"
class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;
	void SetNewArriveRadius(float radius) { m_ArriveRadius = radius; }
	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	float m_ArriveRadius = 0.f;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

/////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
public:
	float m_FleeRadius = 25.f;

};

/////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Seek Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	float _speed = 5.f;
};


/////////////////////////
//WANDER
//****
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetWanderOffset(float offset) { m_OffSetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }
private:
	float m_OffSetDistance = 6.f;
	float m_Radius = 4.f;
	float m_MaxAngleChange = 15.f;
	float m_WanderAngle = 0.f;
};

class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	inline void SetSlowRadius(float distance) { m_Distance = distance; }
private:
	float m_Distance = 10;
	float m_OriginalDistance;
	Elite::Vector2 m_OriginalGoal;
};
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	float m_Range = 5.f;
	float m_MaxDistance = 1.f;
	bool m_IsPerpendicular = false;
	Elite::Vector2 m_PreviousTargetPos;
	float m_DistanceMultiplier = 3.f;

};
class Evade : public Seek
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:

	float m_EvadeDistance = 5.f;
	Elite::Vector2 m_PreviousTargetPos;

};
#endif


