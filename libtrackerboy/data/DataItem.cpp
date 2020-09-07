
#include "trackerboy/data/DataItem.hpp"

#include "./checkedstream.hpp"

namespace trackerboy {

DataItem::DataItem() :
    mId(0),
    mName()
{
}

DataItem::~DataItem() {

}

uint8_t DataItem::id() const noexcept {
    return mId;
}

std::string DataItem::name() const noexcept {
    return mName;
}

void DataItem::setId(uint8_t id) noexcept {
    mId = id;
}

void DataItem::setName(std::string name) noexcept {
    mName = name;
}

FormatError DataItem::serialize(std::ostream &stream) noexcept {
    // id
    checkedWrite(stream, &mId, sizeof(mId));

    // name
    checkedWrite(stream, mName.c_str(), mName.size() + 1);

    // payload (implementation-specific)
    return serializeData(stream);
}

FormatError DataItem::deserialize(std::istream &stream) noexcept {
    // id
    checkedRead(stream, &mId, sizeof(mId));
    
    // name
    std::getline(stream, mName, '\0');
    if (!stream.good()) {
        return FormatError::readError;
    }

    // payload
    return deserializeData(stream);
}


}
