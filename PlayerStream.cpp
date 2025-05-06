#include "PlayerStream.hpp"

VectorPlayerStream::VectorPlayerStream(const std::vector<Player>& players)
: players_(players), currentIndex_(0){}