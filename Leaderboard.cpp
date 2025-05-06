#include "Leaderboard.hpp"
#include <chrono>
#include <algorithm>
#include<cmath>

/**
 * @brief Constructor for RankingResult with top players, cutoffs, and elapsed time.
 *
 * @param top Vector of top-ranked Player objects, in sorted order.
 * @param cutoffs Map of player count thresholds to minimum level cutoffs.
 *   NOTE: This is only ever non-empty for Online::rankIncoming().
 *         This parameter & the corresponding member should be empty
 *         for all Offline algorithms.
 * @param elapsed Time taken to calculate the ranking, in seconds.
 */
RankingResult::RankingResult(const std::vector<Player>& top, const std::unordered_map<size_t, size_t>& cutoffs, double elapsed)
    : top_ { top }
    , cutoffs_ { cutoffs }
    , elapsed_ { elapsed }
{
}

RankingResult Offline::heapRank(std::vector<Player>& players){
    auto start = std::chrono::high_resolution_clock::now();

    //ten percent of players
    size_t tenPer = std::floor(0.1 * players.size());

    //make vector into max heap
    std::make_heap(players.begin(), players.end());

    //pop 10 percent of players 
    for(size_t i = 0; i < tenPer; ++i){
        std::pop_heap(players.begin(), players.end() - i);
    }
    //get index of where top ten percent of players start
    auto topStart = players.end() - tenPer;

    auto end = std::chrono::high_resolution_clock::now();
    
    //duration of sorting (ms)
    double elapsed_ = std::chrono::duration<double, std::milli>(end-start).count();
    
    //vector of top ten percent of players sorted 
    std::vector<Player> top_(topStart, players.end());
    //empty cutoffs
    std::unordered_map<size_t, size_t> cutoffs_ = {};

    return RankingResult(top_, cutoffs_, elapsed_);
}

//Lomuto partition function
int Offline::partition(std::vector<Player>& players, int low, int high){
    Player pivot =  players[high];//pivot is the last element
    int i = low - 1; //index of rightmost element < pivot

    for(int it = low; it <=high - 1; it++){
        //compare player levels
        if(players[it].level_ >= pivot.level_){
            i++;
            std::swap(players[i], players[it]);

        }
    }
    std::swap(players[i + 1], players[high]);
    return i + 1; //final pivot position
}

void Offline::quickSelect(std::vector<Player>& players, int left, int right, int cutoffIndex){
    while(left <= right){
        int pivotIndex = partition(players, left, right);

        //pivot is at the cutoff for top ten percent, we're done
        if(pivotIndex == cutoffIndex){
            return;
        }
        //if pivot is too far right, top 10 percent in on left
        else if(pivotIndex > cutoffIndex){
            right = pivotIndex - 1;
        }
        //pivot is too far left, move to right sublist
        else{   
            left = pivotIndex + 1;
        }
    }
}

void Offline::quickSort(std::vector<Player>& players, int low, int high){
    if(low < high){
        //get partition's final position
        int part = partition(players, low, high);
        //sort the right side
        quickSort(players, low, part - 1);
        //sort the left side
        quickSort(players, part + 1, high);
    }
}

RankingResult Offline::quickSelectRank(std::vector<Player>& players){
    auto start = std::chrono::high_resolution_clock::now();

    size_t tenPer = std::floor(0.1 * players.size());
    size_t cutoffIndex = players.size() - tenPer;   //start at top ten percent

    //select top ten percent using quickselect
    quickSelect(players, 0, players.size() - 1, cutoffIndex);

    //sort the top ten percent 
    //quickSort(players, cutoffIndex, players.size() - 1);
    std::sort(players.begin() + cutoffIndex, players.end());
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    //put top ten percent in vector
    std::vector<Player> topPlayers(players.begin() + cutoffIndex, players.end());
    
    std::unordered_map<size_t, size_t> cutoffs_ = {};


    return RankingResult(topPlayers, cutoffs_, elapsed);
}

void Online::replaceMin(PlayerIt first, PlayerIt last, Player& target){
    //replace the root (min element) with target
    *first = std::move(target);

    auto heapSize = std::distance(first, last);
    auto currentPlayer = 0;     //start at root

    while(true){
        auto left = 2 * currentPlayer + 1; //left child index
        auto right = 2 * currentPlayer + 2;   //right child
        auto smallest = currentPlayer;

        //find the smallest 
        if(left < heapSize && std::next(first,left)->level_ < std::next(first,smallest)->level_){
            smallest = left;
        }
        if(right < heapSize && std::next(first, right)->level_ < std::next(first,smallest)->level_){
            smallest = right;
        }

        //stop if restored the heap property
        if(smallest == currentPlayer){
            return;
        }

        //swap with the smaller child
        std::iter_swap(std::next(first, currentPlayer), std::next(first, smallest));
        currentPlayer = smallest;
    }

}

RankingResult Online::rankIncoming(PlayerStream& stream, const size_t& reporting_interval) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Player> top_players;
    std::unordered_map<size_t, size_t> cutoffs;
    size_t players_processed = 0;

    while (stream.remaining() > 0) {
        Player p = stream.nextPlayer();
        players_processed++;

        if (top_players.size() < reporting_interval) {
            top_players.push_back(std::move(p));
            if (top_players.size() == reporting_interval) {
                std::make_heap(top_players.begin(), top_players.end(), 
                    [](const Player& a, const Player& b) { return a.level_ > b.level_; }); // Min-heap
            }
        } 
        else if (p.level_ > top_players.front().level_) {
            replaceMin(top_players.begin(), top_players.end(), p);
        }

        if (players_processed % reporting_interval == 0) {
            cutoffs[players_processed] = top_players.front().level_;
        }
    }

    if (!top_players.empty()) {
        cutoffs[players_processed] = top_players.front().level_;
    }

    std::sort(top_players.begin(), top_players.end(), 
        [](const Player& a, const Player& b) { return a.level_ < b.level_; });

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(top_players, cutoffs, elapsed);
}
