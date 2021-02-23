#include "Light.h"
using namespace DirectX;


void Light::InitializeLight(vec3 diffuse, vec3 ambient, vec3 direction, vec3 position)
{
	// Initialize local variables.
	m_diffuse = diffuse;
	m_ambient = ambient;
	m_direction = direction;
	XMStoreFloat3(&m_direction, XMVectorScale(XMVector3Normalize(XMLoadFloat3(&direction)), m_spotPower));
	m_position = position;

	// Initialize the spot light variables (needs to be assigned for all lights).
	float radiusInitial = (1 - (m_spotFalloff / 32) + (1.7f * m_spotPower)) / 0.8f;	// Isn't perfectly accurate, but should be good enough.
	m_theta = atanf(radiusInitial / 4.3f);

	// Initialize the view and projection matrices.
	if (m_lightType == (int)LightType::Spot) {
		XMStoreFloat4x4(&m_projMatrix, XMMatrixPerspectiveFovLH(m_theta, 1.0f, 0.01f, 100.0f));
		XMStoreFloat4x4(&m_viewMatrix, XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_direction), XMVectorSet(0, 1, 0, 0)));
	}
	else if (m_lightType == (int)LightType::Directional) {
		XMStoreFloat4x4(&m_projMatrix, XMMatrixOrthographicLH(10.0f, 10.0f, 0.01f, 100.0f));
		XMStoreFloat4x4(&m_viewMatrix, XMMatrixLookToLH(XMVectorScale(XMLoadFloat3(&(m_direction)), -10.0f), XMLoadFloat3(&m_direction), XMVectorSet(0, 1, 0, 0)));
	}
}

Light::Light(vec3 diffuse, vec3 ambient, vec3 directionOrPosition, LightType type)
{
	m_lightType = (int)type;
	if (type == LightType::Spot)
		printf("Incorrect light type! Use other constructor for spot lights.");
	else if(type == LightType::Directional)
		InitializeLight(diffuse, ambient, directionOrPosition, vec3());
	else if(type == LightType::Point)
		InitializeLight(diffuse, ambient, vec3(), directionOrPosition);
}

Light::Light(vec3 diffuse, vec3 ambient, vec3 position, vec3 direction, float spotFalloff, float spotPower)
{
	m_spotFalloff = spotFalloff;
	m_lightType = (int)LightType::Spot;
	m_spotPower = 0.99f + (spotPower / 100.0f);
	InitializeLight(diffuse, ambient, direction, position);
}


void Light::Update(float dt)
{
	if (m_isSwinging) {
		m_swingCurrPercent += dt / (2 * m_swingDuration);
		float t = (cosf(m_swingCurrPercent * 3.1415f) + 1) / 2;		// Approximate gravity using trig.
		float q = m_swingOmega;

		// Slerp
		vec3 tempStoreVec;
		XMStoreFloat3(
			&tempStoreVec,
			XMVectorScale(
				XMVector3Normalize(
					XMVectorAdd(
						XMVectorScale(m_swingInitDir, (XMScalarSin((1.0f - t) * q) / XMScalarSin(q))),
						XMVectorScale(m_swingFinalDir, (XMScalarSin(t * q) / XMScalarSin(q))))),
			m_spotPower));
		m_direction = tempStoreVec;
	}

}

LightShaderInput Light::Output()
{
	LightShaderInput data = {};
	data.diffuseColor = m_diffuse;
	data.ambientColor = m_ambient;
	data.direction = m_direction;
	data.position = m_position;
	data.lightType = m_lightType;
	data.spotFalloff = m_spotFalloff;

	return data;
}

ViewAndProjMatrices Light::GetMatrices()
{
	ViewAndProjMatrices output = {};
	output.Proj = m_projMatrix;		// Proj matrix doesn't ever change.
	
	// If the light is swinging, the view matrix changes.
	if (m_isSwinging == true) {
		if (m_lightType == (int)LightType::Spot) {
			DirectX::XMStoreFloat4x4(&m_viewMatrix, DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&m_position), DirectX::XMLoadFloat3(&m_direction), DirectX::XMVectorSet(0, 1, 0, 0)));
		}
	}
	output.View = m_viewMatrix;

	
	return output;
}

// Convert the spot light to a swinging light. Angle is in degrees.
void Light::ConvertToSwinging(float rel_roll, float rel_pitch, float rel_yaw, float swingTime)
{
	if (m_isSwinging == true) return;
	m_isSwinging = true;

	rel_roll = XMConvertToRadians(rel_roll);
	rel_pitch = XMConvertToRadians(rel_pitch);
	rel_yaw = XMConvertToRadians(rel_yaw);

	// Generate the vectors used to slerp between.
	vec3 dir = m_direction;
	m_swingInitDir = XMLoadFloat3(&dir);
	m_swingFinalDir = XMVector3Rotate( XMLoadFloat3(&dir), XMQuaternionRotationRollPitchYaw(rel_roll, rel_pitch, rel_yaw));

	// Generate the angle between the vectors.
	XMStoreFloat(&m_swingOmega, XMVector3Dot(m_swingInitDir, m_swingFinalDir));
	m_swingOmega = XMScalarACos(m_swingOmega);

	// Print out the values of the  init and final dir for testing
	// vec3 initDir;
	// XMStoreFloat3(&initDir, m_swingInitDir);
	// vec3 finalDir;
	// XMStoreFloat3(&finalDir, m_swingFinalDir);


	m_swingDuration = swingTime;
}

// Code from https://www.geometrictools.com/Documentation/IntersectionBoxCone.pdf
bool Light::IsBoxCollidingSpotlight(BoundingBox box)
{
	// - - - Variables Setup - - -
	// The min/max height of the cone (the beam will never be longer than 10.0f units).
	//float hMin = 0.0f; float hMax = 10.0f;
	//XMVECTOR boxCenter = XMLoadFloat3(&box.Center);
	//XMVECTOR boxExtents = XMLoadFloat3(&box.Extents);

	XMVECTOR conePos = XMLoadFloat3(&m_position);
	XMVECTOR coneDir = XMVector3Normalize(XMLoadFloat3(&m_direction));
	
	vec3 boxCorners[8];
	box.GetCorners(boxCorners);

	float alpha;
	for (int i = 0; i < 8; i++) {
		// Get normalized vector from box point to vertex.
		XMVECTOR V = XMVector3Normalize(XMLoadFloat3(&boxCorners[i]) - conePos);
		// Get the angle between V and D.
		XMStoreFloat(&alpha, XMVectorACosEst(XMVector3Dot(V, coneDir)));
		// If the angle is smaller than m_theta, it is colliding.
		if (alpha < m_theta) return true;
	}
	return false;

	// Everything after here was an attempt at using a completely accurate cone box collision. After spending a few hours coding
	// I wasn't close to done, so I went with the above, which is almost completely accurate.
	// Code from https://www.geometrictools.com/Documentation/IntersectionBoxCone.pdf

	//std::vector<vec2> candidateEdges;
	//vec2 globalEdges[12] = { vec2(0, 1), vec2(1, 3), vec2(2, 3), vec2(0, 2), vec2(4, 5), vec2(5, 7),
	//						 vec2(6, 7), vec2(4, 6), vec2(0, 4), vec2(1, 5), vec2(3, 7), vec2(2, 6) };

	//

	//// - - - Lambda functions - - -
	//auto HasPointInsideCone = [&coneDir, this](XMVECTOR P0, XMVECTOR P1) {
	//	// Define F(X) = Dot(U,X - V)/|X - V|, where U is the unit-length
	//	// cone axis direction and V is the cone vertex.  The incoming
	//	// points P0 and P1 are relative to V; that is, the original
	//	// points are X0 = P0 + V and X1 = P1 + V.  The segment <P0,P1>
	//	// and cone intersect when a segment point X is inside the cone;
	//	// that is, when F(X) > cosAngle.  The comparison is converted to
	//	// an equivalent one that does not involve divisions in order to
	//	// avoid a division by zero if a vertex or edge contain (0,0,0).
	//	// The function is G(X) = Dot(U,X-V) - cosAngle*Length(X-V).


	//	// Test whether P0 or P1 is inside the cone.
	//	XMVECTOR U = coneDir;
	//	float g; XMStoreFloat(&g, XMVector3Dot(U, P0) - (cosf(m_theta) * XMVector3Length(P0)));
	//	if (g > 0.0f)
	//	{
	//		// X0 = P0 + V is inside the cone.
	//		return true;
	//	}

	//	XMStoreFloat(&g, XMVector3Dot(U, P1) - (cosf(m_theta) * XMVector3Length(P1)));
	//	if (g > 0.0f)
	//	{
	//		// X1 = P1 + V is inside the cone.
	//		return true;
	//	}

	//	// Test whether an interior segment point is inside the cone.
	//	XMVECTOR E = P1 - P0;
	//	XMVECTOR crossP0U = XMVector3Cross(P0, U);
	//	XMVECTOR crossP0E = XMVector3Cross(P0, E);
	//	float dphi0; XMStoreFloat(&dphi0, XMVector3Dot(crossP0E, crossP0U));
	//	if (dphi0 > 0.0f)
	//	{
	//		XMVECTOR crossP1U = XMVector3Cross(P1, U);
	//		float dphi1; XMStoreFloat(&dphi1, XMVector3Dot(crossP0E, crossP1U));
	//		if (dphi1 < 0.0f)
	//		{
	//			float t = dphi0 / (dphi0 - dphi1);
	//			XMVECTOR PMax = P0 + t * E;
	//			XMStoreFloat(&g, XMVector3Dot(U, PMax) - cosf(m_theta) * XMVector3Length(PMax));
	//			if (g > 0.0f)
	//			{
	//				// The edge point XMax = Pmax + V is inside the cone.
	//				return true;
	//			}
	//		}
	//	}

	//	return false;
	//
	//};
	//auto CandidatesHavePointInsideCone = [&boxCorners, &candidateEdges, &HasPointInsideCone, this](std::vector<vec2>) {
	//	for (int i = 0; i < candidateEdges.size(); i++)
	//	{
	//		vec2 edge = candidateEdges[i];
	//		XMVECTOR P0 = XMLoadFloat3(&boxCorners[(int)edge.x]);
	//		XMVECTOR P1 = XMLoadFloat3(&boxCorners[(int)edge.y]);
	//		if (HasPointInsideCone(P0, P1))
	//		{
	//			return true;
	//		}
	//	}
	//	return false;
	//	
	//};

	//// - - - Quick rejectance test: box outside of slab - - -
	//XMVECTOR CmV = boxCenter - conePos;
	//float DdCmV; XMStoreFloat(&DdCmV, XMVector3Dot(coneDir, CmV));
	//float radius; XMStoreFloat(&radius, XMVector3Dot(boxExtents, XMVectorAbs(coneDir)));
	//float bMin = DdCmV - radius;
	//float bMax = DdCmV + radius;

	//if (bMax <= hMin || bMin >= hMax) return false;


	//// - - - Quick acceptance test: box intersects center ray - - -
	//if (box.Intersects(conePos, coneDir, hMax))	return true;


	//// - - - Box is fully inside the slab - - -
	//box.GetCorners(boxCorners);
	//for (int i = 0; i < 8; i++) {
	//	XMStoreFloat3(&boxCorners[i], XMLoadFloat3(&boxCorners[i]) - conePos);
	//}
	//if (hMin <= bMin && hMax >= bMax) {
	//	for (int i = 0; i < 12; i++) {
	//		candidateEdges[i] = globalEdges[i];
	//	}
	//	return CandidatesHavePointInsideCone(candidateEdges);
	//}

	//return false;
}

//bool Light::IsTriangleCollidingSpotlight(XMVECTOR A0, XMVECTOR A1, XMVECTOR A2)
//{
//	// Calculate the radius of the light projected onto the ground.
//	float h = 3.3f;
//	float theta = m_swingOmega / 2;
//	float s = m_spotFalloff;
//	float p = m_spotPower;
//
//	float radiusInitial = 1 - (s / 32) + ((p - 7) / 100);	// Isn't perfectly accurate, but should be good enough.
//	float alpha = atanf(radiusInitial / h);
//	float radiusCurrent = h * (tanf(theta + alpha) - tanf(theta));
//
//	// Using the current position, h, and radiusCurrent, get the three points of a triangle, and check against the given points.
//	vec3 height = vec3(0.0f, -h, 0.0f);
//	vec3 length = vec3(radiusCurrent, 0.0f, 0.0f);
//	for (int i = 0; i < 8; i++) {
//		XMVECTOR B0 = XMLoadFloat3(&m_position);
//		XMVECTOR B1 = XMVectorAdd(B0, XMLoadFloat3(&height));
//		XMVECTOR B2 = XMVectorAdd(B1, XMVector3Rotate(XMLoadFloat3(&length), XMQuaternionRotationRollPitchYaw(0.0f, XM_PIDIV4 * i, 0.0f)));
//
//		if (TriangleTests::Intersects(A0, A1, A2, B0, B1, B2))
//			return true;
//	}
//	return false;
//}
