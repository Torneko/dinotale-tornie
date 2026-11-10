#pragma once
// The native producer and book player never hold each other's audio mutex.
// Claim before pumping the book; stop the book before releasing this gate.
void workshop_menu_native_audio_gate0310(bool muted) noexcept;
