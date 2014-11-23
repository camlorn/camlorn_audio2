/**Copyright (C) Austin Hicks, 2014
This file is part of Libaudioverse, a library for 3D and environmental audio simulation, and is released under the terms of the Gnu General Public License Version 3 or (at your option) any later version.
A copy of the GPL, as well as other important copyright and licensing information, may be found in the file 'LICENSE' in the root of the Libaudioverse repository.  Should this file be missing or unavailable to you, see <http://www.gnu.org/licenses/>.*/
#include <libaudioverse/libaudioverse.h>
#include <libaudioverse/libaudioverse_properties.h>
#include <libaudioverse/private_simulation.hpp>
#include <libaudioverse/private_resampler.hpp>
#include <libaudioverse/private_objects.hpp>
#include <libaudioverse/private_properties.hpp>
#include <libaudioverse/private_macros.hpp>
#include <libaudioverse/private_memory.hpp>
#include <libaudioverse/private_kernels.hpp>
#include <limits>
#include <memory>
#include <algorithm>
#include <utility>
#include <vector>
#include <lambdatask/threadsafe_queue.hpp>

class LavPullObject: public LavObject {
	public:
	LavPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels);
	void process();
	unsigned int input_sr = 0, channels = 0;
	std::shared_ptr<LavResampler> resampler = nullptr;
	float* incoming_buffer = nullptr, *resampled_buffer = nullptr;
	LavPullObjectAudioCallback callback = nullptr;
	void* callback_userdata = nullptr;
};

LavPullObject::LavPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels): LavObject(Lav_OBJTYPE_PULL, sim, 0, channels) {
	this->channels = channels;
	input_sr = inputSr;
	resampler = std::make_shared<LavResampler>(sim->getBlockSize(), channels, inputSr, (int)sim->getSr());
	this->channels = channels;
	incoming_buffer = new float[channels*simulation->getBlockSize()]();
	resampled_buffer = new float[channels*sim->getBlockSize()]();
}

std::shared_ptr<LavObject> createPullObject(std::shared_ptr<LavSimulation> sim, unsigned int inputSr, unsigned int channels) {
	return std::make_shared<LavPullObject>(sim, inputSr, channels);
}

void LavPullObject::process() {
	//first get audio into the resampler if needed.
	unsigned int got = 0;
	while(got < block_size) {
		got += resampler->write(resampled_buffer, block_size-got);
		if(got >= block_size) break; //we may have done it on this iteration.
		if(callback) {
			callback(this, block_size, channels, incoming_buffer, callback_userdata);
		} else {
			memset(incoming_buffer, 0, block_size*sizeof(float)*channels);
		}
		resampler->read(incoming_buffer);
	}
	//this is simply uninterweaving, but taking advantage of the fact that we have a different output destination.
	for(unsigned int i = 0; i < block_size*channels; i+=channels) {
		for(unsigned int j = 0; j < channels; j++) {
			outputs[j][i/channels] = resampled_buffer[i+j];
		}
	}
}

//begin public api.

Lav_PUBLIC_FUNCTION LavError Lav_createPullObject(LavSimulation* simulation, unsigned int sr, unsigned int channels, LavObject** destination) {
	PUB_BEGIN
	LOCK(*simulation);
	*destination = outgoingPointer<LavObject>(createPullObject(incomingPointer<LavSimulation>(simulation), sr, channels));
	PUB_END
}

Lav_PUBLIC_FUNCTION LavError Lav_pullObjectSetAudioCallback(LavObject* object, LavPullObjectAudioCallback callback, void* userdata) {
	PUB_BEGIN
	LOCK(*object);
	if(object->getType() != Lav_OBJTYPE_PULL) throw LavErrorException(Lav_ERROR_TYPE_MISMATCH);
	((LavPullObject*)object)->callback = callback;
	((LavPullObject*)object)->callback_userdata = userdata;
	PUB_END
}