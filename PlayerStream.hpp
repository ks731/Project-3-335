#pragma once
#include "Player.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Uncomment the following if you're doing extra credit!
// Otherwise, feel free to ignore this or erase APIPlayerStream.
// #define API_ENABLED

/**
 * @brief Interface for fetching Player objects sequentially.
 */
class PlayerStream {
public:
    /**
     * @brief Retrieves the next Player in the stream, if possible.
     *
     * @return The next Player object in the sequence.
     * @post Updates stream members so a subsequent call to
     *      nextPlayer() will yield the next Player in the to-be-read
     *      sequence.
     *
     * @throws std::runtime_error, if there are no more players to fetch
     *      & nextPlayer() is called.
     */

    virtual Player nextPlayer() = 0;

    /**
     * @brief Returns the number of players remaining in the stream.

     * @return The count of players left to be read.
     */
    virtual size_t remaining() const = 0;
};

/**
 * @brief The interface for a PlayerStream created using the contents of a vector.
 *
 * @example Let's cover a brief example:
 * Given a vector of Player objects v = {
 *      Player("Rykard", 23),
 *      Player("Malenia", 99)
 *  }
 *
 * Our stream's behavior would be someting akin to:
 *
 * stream.remaining() -> 2
 * stream.nextPlayer() -> Player("Rykard", 23)
 * stream.nextPlayer() -> Player("Malenia", 99)
 * stream.remaining() -> 0
 * stream.nextPlayer() -> throws std::runtime_error()
 */
class VectorPlayerStream : public PlayerStream {
private:
    // Your private members here. You're the designer now!
    std::vector<Player> players_;
    size_t currentIndex_;

public:
    /**
     * @brief Constructs a VectorPlayerStream from a vector of Players.
     *
     * Initializes the stream with a sequence of Player objects matching the
     * contents of the given vector.
     *
     * @param players The vector of Player objects to stream.
     */
    VectorPlayerStream(const std::vector<Player>& players);

    /**
    * @brief Retrieves the next Player in the stream.
    *
    * @return The next Player object in the sequence.
    * @post Updates members so a subsequent call to nextPlayer() yields the Player
    * following that which is returned.

    * @throws std::runtime_error If there are no more players remaining in the stream.
    */
    Player nextPlayer() override;

    /**
     * @brief Returns the number of players remaining in the stream.
     *
     * @return The count of players left to be read.
     */
    size_t remaining() const override; // see how many instances remaining to be fetched
};

// For extra credit
#ifdef API_ENABLED
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

/**
 * @brief A PlayerStream implementation that fetches Player objects from an API in batches.
 *
 * Specifically, this retrieves Player objects localhost (127.0.0.1) at port 5000 using HTTP requests
 * to fetch subequent batches of Players of specified size.
 *
 * Calls fetch *batches* of players--it'd be inefficient to keep calling the API for EVERY single players,
 * when we could possibly hundreds or thousands!
 *
 * Make calls by querying the following endpoint, while running the sample server distributed with your code:
 * http://127.0.0.1:5000/api?seed={POSITIVE NUMBER}&cursor={CURSOR}&batch={BATCH_SIZE}
 *
 * So an example might be :
 * http://127.0.0.1:5000/api?seed=1&cursor=1&batch=5
 *
 * Which might return a JSON whose body is of the form (note the numbers won't matched up with yours):
 * {
 *  cursor: 6,
 *  levels: [ 10, 20, 23, 1, 389 ]
 * }
 *
 * NOTE:
 * 1) You'll have to use the cpr library to call the API
 *    via cpr::Get & cpr::Url
 * 2) And parse the JSON response using the nlohmann::json library.
 *    via json::parse & `.template get<size_t>()`
 *
 * See:
 * 1) https://docs.libcpr.org/introduction.html
 * 2) https://json.nlohmann.me/api/basic_json/get/#examples
 *
 * On you to read the docs! It's extra credit after all!
 * We've also included a sample server that will match the
 * format of the server of what you'll be calling.
 *
 * The numbers will be different though, so don't even think of hard-coding it :)
 */
class APIPlayerStream : public PlayerStream {
protected:
    const std::string PORT = "5000";
    const std::string HOSTNAME = 'http://127.0.0.1';
    const std::string SOCKET = HOSTNAME + ":" + PORT;

private:
    /**
     * @brief
     * 1) Imagine you're querying from a database and there's millions of results
     * Instead, of returning all million of them, we'll periodically fetch a batch of them,
     * and store where in that sequence of million results we're at.
     * `cursor_` is *exactly* this. Think of it like page number on Google search.
     *
     * 2) Since we're not working with an actual database,
     * we'll use seeds to pseudo-randomly generate contents.
     * Make sure to include this in your calls.
     */
    size_t cursor_, seed_;

    // Your additional private members here
public:
    /**
     * @brief Constructs an APIPlayerStream that fetches Players from an API and presents the contents as a stream
     *
     * @param expected_length The total number of Player objects expected from the API.
     * @param seed A seed value used for API requests to ensure consistent results
     * @param batch_size The number of Player objects to fetch in each API request.
     *
     * @details For simplicitly, assume that expected_length is a multiple of batch_size
     *
     * @post
     * a) `cursor_` is initialized to 1
     * b) `seed_` is initialized to the provided seed value.
     * c) You're the designer for the rest, create private member variables
     *    to achieve the outlined functionality.
     */
    APIPlayerStream(const size_t& expected_length, const size_t& seed, const size_t& batch_size = 5);

    /**
    * @brief Retrieves the next Player in the stream.
    *
    * @details Performs either of the following:
    * If the currently stored batched has players that have not been read yet,
    * returns the next Player object from the current batch.
    *
    * OR
    *
    * If the current batch has been exhausted,
    *   1) Issues a new API request
    *       using 'seed_' for the seed query parameter
    *       the current `cursor_` value for the cursor query parameter
    *       the batch size specified in the constructor for the batch query parameter
    *   2) Records the fetched batch contents;
    *       since the task at hand is only related to ranking levels,
    *       the API will ONLY return a number representing the level of a player.
    *       When recording the contents, you can name Player objects whatever you like.
    *
    *   3) Returns the first Player Object in the newly retrieved batch.

    * @post If a new API call is made, `cursor_` is
    *   set to the updated cursor value returned by the API call.
    *
    * @return The next Player object in the sequence.
    * @throws std::runtime_error If there are no more players remaining or if the API request fails.

    */
    Player nextPlayer() override;

    /**
     * @brief Returns the number of players remaining in the stream.
     *
     * @return The count of players left to be read.
     * @example If our stream is initialized with expected length 5,
     *   after calling nextPlayer() twice, we'll have 3 players remaining.
     */
    size_t remaining() const override;
};

#endif