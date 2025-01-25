#include "Contact.h"

namespace phy
{
	void ResolveContact(contact_t& contact)
	{
		Body* A = contact.bodyA;
		Body* B = contact.bodyB;

		const float invMassA = A->m_invMass;
		const float invMassB = B->m_invMass;

		const float elasticityA = A->m_elasticity;
		const float elasticityB = B->m_elasticity;
		const float elasticity = elasticityA * elasticityB;

		//Calculate Collision Imact
		const Vec3& n = contact.normal;
		const Vec3& vab = A->m_LinearVelocity - B->m_LinearVelocity;

		const float ImpulseJ = -(1.0f + elasticity) * vab.Dot(n) / (invMassA + invMassB);
		const Vec3 vectorImpulseJ = n * ImpulseJ;

		A->ApplyImpulseLinear(vectorImpulseJ * 1.0f);
		B->ApplyImpulseLinear(vectorImpulseJ * -1.0f);


	}
}