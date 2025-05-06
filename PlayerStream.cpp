#include "PlayerStream.hpp"

VectorPlayerStream::VectorPlayerStream(const std::vector<Player>& players)
: players_(players), currentIndex_(0){}

Player VectorPlayerStream::nextPlayer(){
    if(currentIndex_ >= players_.size()){
        throw std::runtime_error("No more players in stream");
    }
    return players_[currentIndex_++];  //move to next player
}

size_t VectorPlayerStream::remaining()const{
    return players_.size() - currentIndex_;
}
