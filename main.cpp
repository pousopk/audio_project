#include <RtAudio.h>

#include <atomic>
#include <cmath>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>

struct MetronomeState {
	double sampleRate = 48000.0;
	std::atomic<double> bpm{120.0};
	double volume = 0.7;
	double phase = 0.0;
	int samplesUntilBeat = 0;
	int clickSamplesRemaining = 0;
	int beatIndex = 0;
	int beatsPerBar = 4;
	double clickFreqStrong = 1760.0;
	double clickFreqWeak = 1320.0;
	double clickDurationSec = 0.03;
};

static std::atomic<bool> gRunning{true};

void handleSignal(int) { gRunning = false; }

int audioCallback(void* outputBuffer,
									void* /*inputBuffer*/,
									unsigned int nBufferFrames,
									double /*streamTime*/,
									RtAudioStreamStatus status,
									void* userData) {
	auto* state = static_cast<MetronomeState*>(userData);
	auto* out = static_cast<float*>(outputBuffer);

	if (status) {
		std::cerr << "Stream under/overrun detected.\n";
	}

	const double twoPi = 6.28318530717958647692;
	const int clickLenSamples = static_cast<int>(state->clickDurationSec * state->sampleRate);

	for (unsigned int i = 0; i < nBufferFrames; ++i) {
		if (state->samplesUntilBeat <= 0) {
			state->samplesUntilBeat = static_cast<int>((60.0 / state->bpm.load()) * state->sampleRate);
			state->clickSamplesRemaining = clickLenSamples;
			state->beatIndex = (state->beatIndex + 1) % state->beatsPerBar;
		}

		float sample = 0.0f;
		if (state->clickSamplesRemaining > 0) {
			const bool strongBeat = (state->beatIndex == 1);
			const double freq = strongBeat ? state->clickFreqStrong : state->clickFreqWeak;
			const double env = static_cast<double>(state->clickSamplesRemaining) / clickLenSamples;
			sample = static_cast<float>(state->volume * env * std::sin(state->phase));
			state->phase += twoPi * freq / state->sampleRate;
			if (state->phase > twoPi) state->phase -= twoPi;
			--state->clickSamplesRemaining;
		}

		out[i] = sample;
		--state->samplesUntilBeat;
	}

	return gRunning ? 0 : 1;
}

int main(int argc, char** argv) {
	MetronomeState state;

	if (argc > 1) {
		const double parsed = std::atof(argv[1]);
		if (parsed > 20.0 && parsed < 400.0) {
			state.bpm = parsed;
		}
	}

	std::signal(SIGINT, handleSignal);

	try {
		RtAudio dac;
		if (dac.getDeviceCount() < 1) {
			std::cerr << "No audio output device found.\n";
			return 1;
		}

		RtAudio::StreamParameters params;
		params.deviceId = dac.getDefaultOutputDevice();
		params.nChannels = 1;
		params.firstChannel = 0;

		unsigned int bufferFrames = 256;
		unsigned int sampleRate = 48000;
		state.sampleRate = static_cast<double>(sampleRate);
		state.samplesUntilBeat = 0;

		dac.openStream(&params,
									 nullptr,
									 RTAUDIO_FLOAT32,
									 sampleRate,
									 &bufferFrames,
									 &audioCallback,
									 &state);
		dac.startStream();

		std::cout << "Metronome running at " << state.bpm.load()
							<< " BPM. Press Ctrl+C to stop.\n";

		while (gRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (dac.isStreamRunning()) dac.stopStream();
		if (dac.isStreamOpen()) dac.closeStream();
	}catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}
