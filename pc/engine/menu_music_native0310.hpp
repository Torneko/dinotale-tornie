#pragma once
// Process contract for the CPU-only original title-music renderer.
// No functions from the game host are linked or initialized by this worker.
namespace dinotale::menu_music_native0310 {
inline constexpr const char* original_usa_rom_sha256 = "e8a63388c38f8f0bea37e9b13a97f0898ac5496e08ca2028afb8db9a866e0ce9";
inline constexpr unsigned logical_track = 22;
inline constexpr unsigned physical_sequence = 11;
inline constexpr unsigned render_vi_count = 9500;
inline constexpr unsigned max_render_vi_count = 20000;
inline constexpr unsigned max_pcm_frames = 11000000;
// CLI: MusicRender0310 ROM.z64|v64|n64 OUTPUT.wav [diagnostic_vi_count]
// OUTPUT.wav.json schema=1: normalized_rom_sha256, renderer_sha256,
// sample_rate, channels=2, bits_per_sample=16, frames, loop_begin, loop_end.
// PCM is untouched original AI output, stereo L/R, before host volume.
// --identity writes the source-bound renderer SHA256 without reading a ROM.
}
