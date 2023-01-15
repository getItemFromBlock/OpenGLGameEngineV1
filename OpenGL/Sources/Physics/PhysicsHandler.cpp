#include "Physics/PhysicsHandler.hpp"

#include <ImGUI/imgui.h>

#include "Core/Util/TextHelper.hpp"

#include "Core/DataStructure/Node.hpp"
#include "Physics/Colliders/Collider.hpp"
#include "Physics/Colliders/BallCollider.hpp"
#include "Physics/Colliders/BoxCollider.hpp"
#include "Physics/Colliders/CapsuleCollider.hpp"
#include "Physics/RigidBody.hpp"

#define SPHERE_SPHERE 0x11
#define SPHERE_CUBE 0x21
#define CUBE_SPHERE 0x12
#define CUBE_CUBE 0x22

using namespace Core::Maths;

void Physics::PhysicsHandler::UpdatePhysics(Core::DataStructure::SceneNode* scene, bool PlayMode, float deltaTime)
{
	colliders.clear();
	scene->UpdatePhysics(&colliders, PlayMode ? deltaTime : 0.0f);
	if (!PlayMode) return;
	for (auto i = colliders.begin(); i != colliders.end(); i++)
	{
		if ((*i)->Type == Colliders::ColliderType::None) continue;
		for (auto j = i+1; j != colliders.end(); j++)
		{
			if ((*j)->Type == Colliders::ColliderType::None) continue;
			if ((*i) != (*j) && !((*i)->AttachedBody != nullptr && (*i)->AttachedBody == (*j)->AttachedBody) && ((*i)->LayerMask & (*j)->LayerMask))
			{
				Vec3D PosA = (*i)->ModelMatrix.GetPositionFromTranslation();
				Vec3D PosB = (*j)->ModelMatrix.GetPositionFromTranslation();
				if ((PosA - PosB).lengthSquared() < powf((*i)->Range + (*j)->Range, 2) + 0.5f)
				{
					CollideObjects(*i, *j, deltaTime);
				}
			}
		}
	}
}

bool Physics::PhysicsHandler::CollideObjects(Colliders::Collider* a, Colliders::Collider* b, float deltaTime)
{
	int CollisonType = (int)a->Type + ((int)b->Type << 4);
	switch (CollisonType)
	{
	case SPHERE_SPHERE:
		return HandleSphereSphere(a,b, deltaTime);
	case CUBE_SPHERE:
		std::swap(a,b);
		[[fallthrough]]; // Removes warning
	case SPHERE_CUBE:
		return HandleSphereCube(a, b, deltaTime);
	default:
		return false;
	}
}

bool Physics::PhysicsHandler::HandleSphereSphere(Colliders::Collider* a, Colliders::Collider* b, float deltaTime)
{
	Colliders::BallCollider* c1 = static_cast<Colliders::BallCollider*>(a);
	Colliders::BallCollider* c2 = static_cast<Colliders::BallCollider*>(b);
	Vec3D PosA = c1->ModelMatrix.GetPositionFromTranslation();
	Vec3D PosB = c2->ModelMatrix.GetPositionFromTranslation();
	Vec3D MidVec = PosA - PosB;
	float distance = MidVec.getLength();
	if (distance <= (c1->Range + c2->Range))
	{
		a->Entered = true;
		b->Entered = true;
		if (!c1->IsTrigger && !c2->IsTrigger)
		{
			if (a->AttachedBody)
			{
				if (b->AttachedBody)
				{
					Mat4D LocalMatrix = GetLocalMatrix(a->AttachedBody->parent);
					Mat4D ParentMatA = Mat4D::CreateRotationMatrix(a->AttachedBody->parent->GetLocalRotation());
					Vec3D LocalBPos = (LocalMatrix.CreateInverseMatrix() * Vec4D(PosB)).getVector();
					Vec3D AB = LocalBPos - c1->Position;
					Vec3D ImpactPointA = AB.unitVector() * c1->Range + c1->Position;
					float Ratio = a->AttachedBody->Mass / (a->AttachedBody->Mass + b->AttachedBody->Mass);
					a->AttachedBody->parent->SetLocalPosition(a->AttachedBody->parent->GetLocalPosition() - (ParentMatA * AB).getVector().unitVector() * ((c1->Range + c2->Range) - distance) * (1 - Ratio));
					
					LocalMatrix = GetLocalMatrix(b->AttachedBody->parent);
					Mat4D ParentMatB = Mat4D::CreateRotationMatrix(b->AttachedBody->parent->GetLocalRotation());
					Vec3D LocalAPos = (LocalMatrix.CreateInverseMatrix() * Vec4D(PosA)).getVector();
					Vec3D BA = LocalAPos - c2->Position;
					b->AttachedBody->parent->SetLocalPosition(b->AttachedBody->parent->GetLocalPosition() - (ParentMatB * BA).getVector().unitVector() * ((c1->Range + c2->Range) - distance) * Ratio);
					Vec3D ImpactPointB = BA.unitVector() * c2->Range + c2->Position;
					ApplyReactionDouble(a,b,ImpactPointA, ImpactPointB, AB, BA, ParentMatA, ParentMatB, deltaTime);
				}
				else
				{
					Mat4D LocalMatrix = GetLocalMatrix(a->AttachedBody->parent);
					Mat4D ParentMat = Mat4D::CreateRotationMatrix(a->AttachedBody->parent->GetLocalRotation());
					Vec3D LocalBPos = (LocalMatrix.CreateInverseMatrix() * Vec4D(PosB)).getVector();
					Vec3D AB = LocalBPos - c1->Position;
					a->AttachedBody->parent->SetLocalPosition(a->AttachedBody->parent->GetLocalPosition() - (ParentMat * AB).getVector().unitVector() * ((c1->Range + c2->Range) - distance));
					Vec3D ImpactPoint = AB.unitVector() * c1->Range + c1->Position;
					ApplyReaction(a, b, ImpactPoint, AB, ParentMat, deltaTime);
				}
			}
			else if (b->AttachedBody)
			{
				Mat4D LocalMatrix = GetLocalMatrix(b->AttachedBody->parent);
				Mat4D ParentMat = Mat4D::CreateRotationMatrix(b->AttachedBody->parent->GetLocalRotation());
				Vec3D LocalAPos = (LocalMatrix.CreateInverseMatrix() * Vec4D(PosA)).getVector();
				Vec3D BA = LocalAPos - c2->Position;
				b->AttachedBody->parent->SetLocalPosition(b->AttachedBody->parent->GetLocalPosition() - (ParentMat * BA).getVector().unitVector() * ((c1->Range + c2->Range) - distance));
				Vec3D ImpactPoint = BA.unitVector() * c2->Range + c2->Position;
				ApplyReaction(b, a, ImpactPoint, BA, ParentMat, deltaTime);
			}
		}
		else
		{
			NotifyTriggers(a, b);
		}
		return true;
	}
	return false;
}

bool Physics::PhysicsHandler::HandleSphereCube(Colliders::Collider* a, Colliders::Collider* b, float deltaTime)
{
	Colliders::BallCollider* c1 = static_cast<Colliders::BallCollider*>(a);
	Colliders::BoxCollider* c2 = static_cast<Colliders::BoxCollider*>(b);
	Vec3D PosA = c1->ModelMatrix.GetPositionFromTranslation();
	Vec3D PosB = c2->ModelMatrix.GetPositionFromTranslation();
	Vec3D MidVec = PosA - PosB;
	char EdgeCount = 0;
	Vec3D tmpDistances = Vec3D();
	float distances[6] = { 0.0f };
	Mat4D CubeInv = c2->LocalMatrix.CreateInverseMatrix();
	Vec3D SpherePosR = (CubeInv * PosA).getVector();
	for (char i = 0; i < 6; i++)
	{
		distances[i] = i > 2 ? -SpherePosR[i % 3] - c2->Size[i % 3] / 2 : (SpherePosR[i % 3] - c2->Size[i % 3] / 2);
		if (distances[i] > c1->Range) return false;
		if (distances[i] > 0)
		{
			if (EdgeCount > 2) return false;
			tmpDistances[EdgeCount] = distances[i];
			if (tmpDistances.getLength() > c1->Range) return false;
			EdgeCount++;
		}
	}
	a->Entered = true;
	b->Entered = true;
	if (!c1->IsTrigger && !c2->IsTrigger)
	{
		Vec3D min = Vec3D(c2->Size.lengthSquared());
		Int3D minInd = Int3D(-1);
		for (int n = 0; n < 3; n++)
		{
			for (int i = 0; i < 6; i++)
			{
				if ((n > 0 && i == minInd[n - 1ll]) || (n > 1 && i == minInd[n - 2ll])) continue;
				float value = c1->Range - distances[i];
				if (value < min[n])
				{
					min[n] = value;
					minInd[n] = i;
				}
			}
		}
		Vec3D ImpactPoint = SpherePosR;
		char tmpEdges = 0;
		do
		{
			ImpactPoint[minInd[tmpEdges] % 3] = c2->Size[minInd[tmpEdges] % 3] / 2 * (minInd[tmpEdges] > 2 ? -1.0f : 1.0f);
			tmpEdges++;
		} while (tmpEdges < EdgeCount);
		if (a->AttachedBody)
		{
			if (b->AttachedBody)
			{
			}
			else
			{
				Mat4D LocalMatrix = GetLocalMatrix(a->AttachedBody->parent);
				Mat4D ParentMat = Mat4D::CreateRotationMatrix(a->AttachedBody->parent->GetLocalRotation());
				Vec3D LocalIPoint = (LocalMatrix.CreateInverseMatrix() * (c2->LocalMatrix * ImpactPoint)).getVector();
				Vec3D AB = LocalIPoint - c1->Position;
				a->AttachedBody->parent->SetLocalPosition(a->AttachedBody->parent->GetLocalPosition() + (ParentMat * AB).getVector() - (ParentMat * AB).getVector().unitVector() * c1->Range);
				ApplyReaction(a, b, LocalIPoint, AB, ParentMat, deltaTime);
			}
		}
		else if (b->AttachedBody)
		{
		}
	}
	else
	{
		NotifyTriggers(a, b);
	}
	return true;
}

void Physics::PhysicsHandler::NotifyTriggers(Colliders::Collider* a, Colliders::Collider* b)
{
	if (a->NotifyChilds && a->parent)
	{
		for (size_t n = 0; n < a->parent->components.size(); n++) a->parent->components[n]->OnTrigger(a, b);
	}
	if (b->NotifyChilds && b->parent)
	{
		for (size_t n = 0; n < b->parent->components.size(); n++) b->parent->components[n]->OnTrigger(b, a);
	}
}

Physics::PhysicsHandler::PhysicsHandler()
{
	CollisionLayerNames[0] = "Default Layer";
	CollisionLayerNames[0].resize(256);
	for (char i = 1; i < 32; i++)
	{
		CollisionLayerNames[i] = "Layer";
		CollisionLayerNames[i].resize(256);
	}
}

Physics::PhysicsHandler::~PhysicsHandler()
{
}

void Physics::PhysicsHandler::RenderGUI()
{
	std::string label = "Layer XX";
	for (char i = 0; i < 32; i++)
	{
		label.data()[6] = i / 10 + '0';
		label.data()[7] = i%10+'0';
		ImGui::Text("Layer %02d :", i);
		ImGui::SameLine();
		ImGui::InputText(label.c_str(), CollisionLayerNames[i].data(), CollisionLayerNames[i].size()-1);
	}
}

void Physics::PhysicsHandler::Serialize(std::ofstream& fileOut)
{
	fileOut << "CollisionLayers " << std::endl;
	for (char i = 0; i < 32; i++)
	{
		Parsing::Shift(fileOut, 1);
		fileOut << "LayerName " << CollisionLayerNames[i].c_str() << std::endl;
	}
	fileOut << "EndLayers " << std::endl;
}

void Physics::PhysicsHandler::Deserialize(const char* data, int64_t& pos, int64_t size, int64_t& line, std::string& err)
{
	if (err.c_str()[0]) return;
	int index = 0;
	while (pos < size)
	{
		if (!Text::compareWord(data, pos, size, "LayerName"))
			break;
		if (index >= 32)
		{
			err = "Too many Layers ! Maximum is 32";
			break;
		}
		pos = Text::skipCharSafe(data, pos, size);
		std::string tmp = Text::getText(data, pos, size);
		if (!tmp.c_str()[0])
		{
			err = "Expected name after \"LayerName\"";
			break;
		}
		CollisionLayerNames[index] = tmp;
		CollisionLayerNames[index].resize(256);
		index++;
		pos = Text::endLine(data, pos, size);
		line++;
	}
	if (err.c_str()[0]) return;
	if (!Text::compareWord(data, pos, size, "EndLayers"))
	{
		err = "Expected \"EndLayers\"";
	}
}

Mat4D Physics::PhysicsHandler::GetLocalMatrix(Core::DataStructure::Node* parent)
{
	const Mat4D* globalMat = parent->GetGlobalMatrix();
	Vec3D scale = globalMat->GetScaleFromTranslation();
	if (scale.x == 0 || scale.y == 0 || scale.z == 0) return Mat4D::Identity();
	return Mat4D::CreateTransformMatrix(globalMat->GetPositionFromTranslation(), globalMat->GetRotationFromTranslation(scale), Vec3D(1));
}

void Physics::PhysicsHandler::ApplyReaction(Colliders::Collider* b, Colliders::Collider* other, Vec3D ImpactPoint, Vec3D Dir, Mat4D ParentMat, float deltaTime)
{
	if (b->NotifyChilds && b->parent)
	{
		for (size_t n = 0; n < b->parent->components.size(); n++) b->parent->components[n]->OnCollision(b, other, ImpactPoint, Dir.unitVector());
	}
	Mat4D ParentInverseMat = ParentMat.CreateInverseMatrix();
	float BouncyNess = b->AttachedBody->Bounciness;
	float Friction = b->AttachedBody->DynamicFriction;
	Vec3D Force = (ParentInverseMat * b->AttachedBody->Velocity).getVector().negate();
	Vec3D ResultForce = (Dir.unitVector() * b->AttachedBody->Velocity.getLength() * Dir.unitVector().dotProduct(Force.unitVector()));
	Vec3D RotResultForce = (ImpactPoint).crossProduct(b->AttachedBody->RotVelocity);
	Vec3D ResultForceL = (ParentMat * ResultForce).getVector();
	RotResultForce = (ParentMat * RotResultForce).getVector() * Util::toRadians(1);
	Vec3D TmpVel = b->AttachedBody->Velocity;
	Vec3D TmpRVel = b->AttachedBody->RotVelocity;
	b->AttachedBody->Velocity = b->AttachedBody->Velocity * (1 - Friction) + ResultForceL * (2 * BouncyNess * (1 + deltaTime / 4)) + RotResultForce * Friction;
	b->AttachedBody->RotVelocity = b->AttachedBody->RotVelocity * (1 - Friction) + (ImpactPoint).crossProduct(ResultForce) * (Util::toDegrees(1) / powf(ImpactPoint.getLength(), 2)) + (ImpactPoint).crossProduct(Force) * (Util::toDegrees(1) / powf(ImpactPoint.getLength(), 2)) * Friction;
	if ((b->AttachedBody->Velocity - TmpVel).getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->Velocity = TmpVel;
	if (b->AttachedBody->Velocity.getLength() < b->AttachedBody->StaticFriction)
	{
		b->AttachedBody->Velocity = Vec3D();
		b->AttachedBody->RotVelocity = Vec3D();
	}
	if ((b->AttachedBody->RotVelocity - TmpRVel).getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->RotVelocity = TmpRVel;
	if (b->AttachedBody->RotVelocity.getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->RotVelocity = Vec3D();

}

void Physics::PhysicsHandler::ApplyReactionDouble(Colliders::Collider* a, Colliders::Collider* b, Core::Maths::Vec3D ImpactPointA, Core::Maths::Vec3D ImpactPointB, Core::Maths::Vec3D DirA, Core::Maths::Vec3D DirB, Core::Maths::Mat4D ParentMatA, Core::Maths::Mat4D ParentMatB, float deltaTime)
{
	if (a->NotifyChilds && a->parent)
	{
		for (size_t n = 0; n < a->parent->components.size(); n++) a->parent->components[n]->OnCollision(a, b, ImpactPointA, DirA.unitVector());
	}
	if (b->NotifyChilds && b->parent)
	{
		for (size_t n = 0; n < b->parent->components.size(); n++) b->parent->components[n]->OnCollision(b, a, ImpactPointB, DirB.unitVector());
	}
	Mat4D ParentInverseMat = ParentMatA.CreateInverseMatrix();
	float BouncyNess = (a->AttachedBody->Bounciness + b->AttachedBody->Bounciness) / 2;
	float Friction = (a->AttachedBody->DynamicFriction * b->AttachedBody->DynamicFriction);
	float Ratio = a->AttachedBody->Mass / (a->AttachedBody->Mass + b->AttachedBody->Mass);
	Vec3D Force = (ParentInverseMat * a->AttachedBody->Velocity).getVector().negate();
	Vec3D ResultForce = (DirA.unitVector() * a->AttachedBody->Velocity.getLength() * DirA.unitVector().dotProduct(Force.unitVector()));
	Vec3D RotResultForce = (ImpactPointA).crossProduct(a->AttachedBody->RotVelocity);
	Vec3D ResultForceL = (ParentMatA * ResultForce).getVector();
	RotResultForce = (ParentMatA * RotResultForce).getVector() * Util::toRadians(1);
	Vec3D TmpVel = a->AttachedBody->Velocity;
	Vec3D TmpVelA = a->AttachedBody->Velocity;
	Vec3D TmpRVel = a->AttachedBody->RotVelocity;
	a->AttachedBody->Velocity = a->AttachedBody->Velocity * (1 - Friction) + b->AttachedBody->Velocity * BouncyNess * (1 - Ratio) * (float)M_PI_2 + ResultForceL * (1 - Ratio) * (2 * BouncyNess * (1 + deltaTime / 4)) + RotResultForce * Friction * (1 - Ratio);
	a->AttachedBody->RotVelocity = a->AttachedBody->RotVelocity * (1 - Friction) * Ratio + (ImpactPointA).crossProduct(ResultForce) * (Util::toDegrees(1) / powf(ImpactPointA.getLength(), 2)) * (1 - Ratio) + (ImpactPointA).crossProduct(Force) * (Util::toDegrees(1) / powf(ImpactPointA.getLength(), 2)) * Friction * (1 - Ratio);
	if ((a->AttachedBody->Velocity - TmpVel).getLength() < a->AttachedBody->StaticFriction) a->AttachedBody->Velocity = TmpVel;
	if (a->AttachedBody->Velocity.getLength() < a->AttachedBody->StaticFriction)
	{
		a->AttachedBody->Velocity = Vec3D();
		a->AttachedBody->RotVelocity = Vec3D();
	}
	if ((a->AttachedBody->RotVelocity - TmpRVel).getLength() < a->AttachedBody->StaticFriction) a->AttachedBody->RotVelocity = TmpRVel;
	if (a->AttachedBody->RotVelocity.getLength() < a->AttachedBody->StaticFriction) a->AttachedBody->RotVelocity = Vec3D();

	ParentInverseMat = ParentMatB.CreateInverseMatrix();
	Force = (ParentInverseMat * b->AttachedBody->Velocity).getVector().negate();
	ResultForce = (DirB.unitVector() * b->AttachedBody->Velocity.getLength() * DirB.unitVector().dotProduct(Force.unitVector()));
	RotResultForce = (ImpactPointB).crossProduct(b->AttachedBody->RotVelocity);
	ResultForceL = (ParentMatB * ResultForce).getVector();
	RotResultForce = (ParentMatB * RotResultForce).getVector() * Util::toRadians(1);
	TmpVel = b->AttachedBody->Velocity;
	TmpRVel = b->AttachedBody->RotVelocity;
	b->AttachedBody->Velocity = b->AttachedBody->Velocity * (1 - Friction) + TmpVelA * BouncyNess * Ratio * (float)M_PI_2 + ResultForceL * Ratio * (2 * BouncyNess * (1 + deltaTime / 4)) + RotResultForce * Friction * Ratio;
	b->AttachedBody->RotVelocity = b->AttachedBody->RotVelocity * (1 - Friction) * (1 - Ratio) + (ImpactPointB).crossProduct(ResultForce) * (Util::toDegrees(1) / powf(ImpactPointB.getLength(), 2)) * Ratio + (ImpactPointB).crossProduct(Force) * (Util::toDegrees(1) / powf(ImpactPointB.getLength(), 2)) * Friction * Ratio;
	if ((b->AttachedBody->Velocity - TmpVel).getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->Velocity = TmpVel;
	if (b->AttachedBody->Velocity.getLength() < b->AttachedBody->StaticFriction)
	{
		b->AttachedBody->Velocity = Vec3D();
		b->AttachedBody->RotVelocity = Vec3D();
	}
	if ((b->AttachedBody->RotVelocity - TmpRVel).getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->RotVelocity = TmpRVel;
	if (b->AttachedBody->RotVelocity.getLength() < b->AttachedBody->StaticFriction) b->AttachedBody->RotVelocity = Vec3D();
}