#include "Body.h"

namespace phy
{

	

	void Body::Delete()
	{
		if (m_shape)
		{
			delete m_shape;
		}
	}
}