#pragma once

#include "Structure.h"
#include "Factory.h"

class SurfaceFactory: public Factory
{
public:
	SurfaceFactory(): Factory(constants::SURFACE_FACTORY, "structures/factory_surface.sprite")
	{
		sprite().play(constants::STRUCTURE_STATE_CONSTRUCTION);
		maxAge(600);
		turnsToBuild(7);
		requiresCHAP(false);

		initFactory();
	}

protected:

	void initFactory() override
	{
		addProduct(PRODUCT_DIGGER);
		addProduct(PRODUCT_DOZER);
		addProduct(PRODUCT_MINER);
	}

private:
	void defineResourceInput() override
	{
		resourcesIn().energy(10);
	}
};