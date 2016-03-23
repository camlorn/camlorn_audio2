/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#pragma once
#include <limits>

#define PI 3.141592653589793
#define WILBRAHAM_GIBBS 0.089392222222222

#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY (std::numeric_limits<float>::infinity())