/* 
 * Copyright (C) 2010-2012 Alex Bikfalvi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#pragma once

#include "NodeIpv4.h"
#include "LayerIpRoute.h"
#include "LayerPimSm.h"

namespace SimLib
{
	class RouterIpv4 : public NodeIpv4
	{
	protected:
		// Layers
		LayerIpRoute*	layerIpRoute;
		LayerIgmp*		layerIgmp;
		LayerPimSm*		layerPimSm;

	public:
		RouterIpv4(
			uint						id,
			SimHandler&					sim,
			AddressIpv4					address,
			uint						numLinks,
			Route&						route,
			std::vector<AddressIpv4>&	rp
			);
		virtual ~RouterIpv4();

		virtual void	Initialize();
		virtual void	Finalize();

		inline double	StatMcastEntriesIgmp() { return this->layerIgmp->StatEntries(); }
		inline double	StatMcastEntriesPimSm() { return this->layerPimSm->StatEntries(); }
	};
}
