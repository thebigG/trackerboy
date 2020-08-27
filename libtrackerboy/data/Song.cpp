
#include "trackerboy/data/Song.hpp"
#include "trackerboy/fileformat.hpp"

#include <cmath>
#include <stdexcept>


namespace trackerboy {


Song::Song() :
    mMaster(64),
    mOrder(),
    mRowsPerBeat(DEFAULT_RPB),
    mTempo(DEFAULT_TEMPO),
    mSpeed(DEFAULT_SPEED)
{
    mOrder.push_back({ 0 });
}

uint8_t Song::rowsPerBeat() {
    return mRowsPerBeat;
}

float Song::tempo() {
    return mTempo;
}

float Song::actualTempo() {
    // fixed point -> floating point
    float speed = static_cast<float>(mSpeed >> 3);
    speed += static_cast<float>(mSpeed & 0x3) / 8.0f;
    return 3600.0f / (speed * mRowsPerBeat);
}

Speed Song::speed() {
    return mSpeed;
}

std::vector<Order>& Song::orders() {
    return mOrder;
}

PatternMaster& Song::patterns() {
    return mMaster;
}

Pattern Song::getPattern(uint8_t orderNo) {
    if (orderNo >= mOrder.size()) {
        throw std::invalid_argument("order does not exist");
    }

    Order &order = mOrder[orderNo];
    return mMaster.getPattern(
        order.track1Id,
        order.track2Id,
        order.track3Id,
        order.track4Id
    );
}

void Song::setRowsPerBeat(uint8_t rowsPerBeat) {
    if (rowsPerBeat == 0) {
        throw std::invalid_argument("Cannot have 0 rows per beat");
    }
    mRowsPerBeat = rowsPerBeat;
   // calcSpeed();
}

void Song::setTempo(float tempo) {
    if (tempo <= 0.0f) {
        throw std::invalid_argument("tempo most be positive and nonzero");
    }
    mTempo = tempo;
    //calcSpeed();
}

void Song::setSpeed() {
    float speed = 3600.0f / (mRowsPerBeat * mTempo);
    // F5.3 so round to nearest 1/8th
    speed = std::roundf(speed * 8) / 8;

    // now convert floating point -> fixed point

    // calculate the integral part
    mSpeed = static_cast<uint8_t>(speed) << 3;
    // calculate the fractional part
    float junk; // we only want the fractional part
    uint8_t fract = static_cast<uint8_t>(std::modf(speed, &junk) * 8);
    mSpeed |= fract;
}

void Song::setSpeed(Speed speed) {
    if (speed == 0) {
        throw std::invalid_argument("speed must be nonzero");
    }
    mSpeed = speed;
}


//void Song::calcSpeed() {
//    float speed = 3600.0f / (mRowsPerBeat * mTempo);
//    // F5.3 so round to nearest 1/8th
//    speed = std::roundf(speed * 8) / 8;
//
//    // now convert floating point -> fixed point
//
//    // calculate the integral part
//    mSpeed = static_cast<uint8_t>(speed) << 3;
//    // calculate the fractional part
//    float junk; // we only want the fractional part
//    uint8_t fract = static_cast<uint8_t>(std::modf(speed, &junk) * 8);
//    mSpeed |= fract;
//}


}