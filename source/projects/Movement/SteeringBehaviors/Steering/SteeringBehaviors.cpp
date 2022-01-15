//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
#include <cmath>
//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 direction = m_Target.Position - pAgent->GetPosition(); //dir
	steering.LinearVelocity = direction.GetNormalized();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior()) {
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, direction.Normalize(), {0,1,0,0.5f}, 0.4f);
	}

	return steering;
}
//****
//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	
	float distance = pAgent->GetPosition().DistanceFast(m_Target.Position);

	if (distance < m_FleeRadius) {
		steering = Seek::CalculateSteering(deltaT, pAgent);
		steering.LinearVelocity = -steering.LinearVelocity;
		steering.IsValid = true;

		return steering;
	}
	else {
		steering.IsValid = false;
		return steering;
	}

	if (pAgent->CanRenderBehavior()) {
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_FleeRadius, Elite::Color{ 1.f,0.f,1.f }, 2.f);
	}
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);
	Elite::Vector2 toTargetVector;

	if (m_Target.Position != Elite::Vector2{ 0,0 }) {

		toTargetVector = (m_Target).Position - pAgent->GetPosition(); //DirToLookAtPoint
		float Angle = std::atan2(toTargetVector.y, toTargetVector.x) - pAgent->GetRotation();



		//reset 90 degree offset
		steering.AngularVelocity = Elite::Clamp(Elite::ToDegrees(Angle + (static_cast<float>(M_PI) / 2.f)), -pAgent->GetMaxAngularSpeed(), +pAgent->GetMaxAngularSpeed());

		}

	if (pAgent->CanRenderBehavior()) {
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), toTargetVector, 1, { 1,0,0,0.5f }, 0.4f);

	}



	
	


	//if (INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eLeft)) {
	//	float angle = Elite::Dot(direction, pAgent->GetLinearVelocity()) / (direction.Magnitude() * pAgent->GetLinearVelocity().Magnitude());
//	std::cout << std::acos(angle) << '\n';
//	steering.AngularVelocity = std::acos(angle);
//}
//
//if (pAgent->CanRenderBehavior()) {
//	DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, direction.Normalize(), { 0,1,0,0.5f }, 0.4f);
//}

return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 pos{ pAgent->GetPosition() };
	float lookAngle = pAgent->GetOrientation() - (static_cast<float>(M_PI) / 2.f);
	Elite::Vector2 LookDir = { std::cos(lookAngle) * m_OffSetDistance, std::sin(lookAngle) * m_OffSetDistance };
	Elite::Vector2 circleCenter = pos + LookDir;

	float newAngle = float((rand() % (int)(m_MaxAngleChange * 2)) - m_MaxAngleChange);
	m_WanderAngle += newAngle;
	Elite::Vector2 newWanderPoint = Elite::Vector2{ std::cos(Elite::ToRadians(m_WanderAngle)) * m_Radius, std::sin(Elite::ToRadians(m_WanderAngle)) * m_Radius };
	m_Target.Position = (newWanderPoint + circleCenter);
	steering = Seek::CalculateSteering(deltaT, pAgent);


	if (pAgent->CanRenderBehavior()) {


		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), LookDir, m_OffSetDistance, { 0,1,1 }, 0.4f);
		DEBUGRENDERER2D->DrawPoint(circleCenter, 5.f, Elite::Color{ 0.f,0.f,1.f,1.f });
		DEBUGRENDERER2D->DrawPoint(circleCenter + newWanderPoint, 5.f, Elite::Color{ 1.f,1.f,1.f,1.f });
		DEBUGRENDERER2D->DrawCircle(Elite::Vector2{ pos.x + LookDir.x, pos.y + LookDir.y }, m_Radius, Elite::Color{ 1.f,1.f,1.f, 1.f }, 0.f);
	}
	return steering;

}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	float currentDistance = Elite::Distance(m_Target.Position, pAgent->GetPosition());

	if (currentDistance > 0.1f) {
		if (currentDistance < m_Distance) { //arriving at target
			steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
			steering.LinearVelocity.Normalize();
			steering.LinearVelocity *= (currentDistance / m_Distance) * pAgent->GetMaxLinearSpeed();
		}
		else { //far from target
			steering = Seek::CalculateSteering(deltaT, pAgent);

		}
	}

	if (pAgent->CanRenderBehavior()) {


		DEBUGRENDERER2D->DrawCircle(m_Target.Position, m_Distance, Elite::Color{ 1.f,1.f,1.f, 1.f }, 0.f);
	}

	return steering;

}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	Elite::Vector2 evaderVelocity;
	Elite::Vector2 futurePos;
	Elite::Vector2 perpendicularSeekPos;
	bool isLeftClosest = false;

	if (m_Target.Position.x != 0 && m_Target.Position.y != 0) { //Target is not equal to 0
		//update evader velocity
		evaderVelocity = m_Target.Position - m_PreviousTargetPos; //dir
		evaderVelocity.Normalize();
		//Get future pos if left/right of Target
		futurePos = (evaderVelocity * m_Range) + m_Target.Position;
		//Check where closest distance left or right perpendicular
		isLeftClosest = Elite::Distance(Elite::Vector2{ -evaderVelocity.y, evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) <
			Elite::Distance(Elite::Vector2{ evaderVelocity.y, -evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) ? true : false;
		//get distance to closest perpendicular * m_distancemultiplieer
		float perpendicularDistance = isLeftClosest ? Elite::Distance(Elite::Vector2{ -evaderVelocity.y, evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) * m_DistanceMultiplier :
			Elite::Distance(Elite::Vector2{ evaderVelocity.y, -evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) * m_DistanceMultiplier;

		//Get perpendicular SearchPos
		perpendicularSeekPos = ((isLeftClosest ? Elite::Vector2{ -evaderVelocity.y, evaderVelocity.x } : Elite::Vector2{ evaderVelocity.y, -evaderVelocity.x }) * perpendicularDistance) + m_Target.Position;

		if (((Elite::Dot((pAgent->GetPosition() - m_Target.Position).GetNormalized(), evaderVelocity) < 0))) {

			//going to the perpendicularSeekPos
			steering.LinearVelocity = perpendicularSeekPos - pAgent->GetPosition();
			steering.LinearVelocity.Normalize();
			steering.LinearVelocity *= (pAgent->GetMaxLinearSpeed() * (perpendicularDistance + 1));
			//std::cout << "seeking perpendicular which is closest: " << isLeftClosest << "dot: " <<(Elite::Dot((pAgent->GetPosition() - m_Target.Position).GetNormalized(), evaderVelocity + m_Target.Position)) << std::endl;
			m_IsPerpendicular = false;
		/*	if (Elite::AreEqual(Elite::Dot((m_Target.Position - pAgent->GetPosition()).GetNormalized(), evaderVelocity + m_Target.Position), 0.f)) {
				m_IsPerpendicular = true;
			}*/
		}
		else {
			//std::cout << "Is perpendicular" << (Elite::Dot((pAgent->GetPosition() - m_Target.Position).GetNormalized(), evaderVelocity + m_Target.Position)) << std::endl;
			m_IsPerpendicular = true;

			//pagent is next to target (left or right of the target)
			//perpendicular seek is done now to move in front of target
		}
		
		

		//check if distance does exceeds maxdistance
		if (m_IsPerpendicular == true) {
			if (Elite::Distance(pAgent->GetPosition(), futurePos) > m_MaxDistance) {
				Elite::Vector2 direction = futurePos - pAgent->GetPosition(); //dir
				steering.LinearVelocity = direction.GetNormalized();
				steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (perpendicularDistance + 1);
				//std::cout << "domme shit\n";
			}

		}
	

		if (m_PreviousTargetPos.x != m_Target.Position.x && m_PreviousTargetPos.y != m_Target.Position.y) { //keeps changing
			m_PreviousTargetPos = m_Target.Position;
		}
	}


	if (pAgent->CanRenderBehavior()) {

		DEBUGRENDERER2D->DrawDirection(m_Target.Position, evaderVelocity, 2, { 0,1,1,1 }, 0.4f);
		DEBUGRENDERER2D->DrawDirection(m_Target.Position, (pAgent->GetPosition() - m_Target.Position), 2, { 0,1,0,1 }, 0.4f);

		DEBUGRENDERER2D->DrawDirection(m_Target.Position, isLeftClosest ? Elite::Vector2{-evaderVelocity.y, evaderVelocity.x} : Elite::Vector2{evaderVelocity.y, -evaderVelocity.x }, 2, { 0,1,1,1 }, 0.4f);
		Elite::Vector2 perpSeekPos =  perpendicularSeekPos - m_Target.Position;
		if(!m_IsPerpendicular)
			DEBUGRENDERER2D->DrawPoint(perpendicularSeekPos, 5.f, Elite::Color{ 1,0,0,1 });
		else
			DEBUGRENDERER2D->DrawPoint(futurePos, 5.f, Elite::Color{ 1,0,0,1 });
		
	/*	DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), perpSeekPos, perpSeekPos.Magnitude(), { 1,0,0,1 }, 0.4f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), perpendicularSeekPos - pAgent->GetPosition(), 2, { 1.f,1.f,1.f,1 }, 0.4f);*/



		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, Elite::Distance(pAgent->GetPosition(), futurePos), { 0,1,0,1 }, 0.4f);

	}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	Elite::Vector2 evaderVelocity;
	evaderVelocity = m_Target.Position - m_PreviousTargetPos; //dir

	//check is in Target Range
	if (Elite::Distance(m_Target.Position, pAgent->GetPosition()) < m_EvadeDistance) {
		//is in range
		Elite::Vector2 closestPerpendicular = Elite::Distance(Elite::Vector2{ -evaderVelocity.y, evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) <
			Elite::Distance(Elite::Vector2{ evaderVelocity.y, -evaderVelocity.x } + m_Target.Position, pAgent->GetPosition()) ? Elite::Vector2{ -evaderVelocity.y, evaderVelocity.x } : Elite::Vector2{ evaderVelocity.y, -evaderVelocity.x };

		steering.LinearVelocity = closestPerpendicular.GetNormalized() * (m_Target.Position - pAgent->GetPosition()).Magnitude();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (m_PreviousTargetPos.x != m_Target.Position.x && m_PreviousTargetPos.y != m_Target.Position.y) { //keeps changing
		m_PreviousTargetPos = m_Target.Position;
	}
	return steering;
}

