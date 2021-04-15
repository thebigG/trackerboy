
#pragma once

#include "trackerboy/data/DataItem.hpp"
#include "trackerboy/data/Instrument.hpp"
#include "trackerboy/data/Waveform.hpp"

#include <cstddef>
#include <memory>
#include <set>
#include <type_traits>
#include <vector>

namespace trackerboy {


class DataListBase {

    using DataVector = std::vector<std::shared_ptr<DataItem>>;
    using IdSet = std::set<uint8_t>;

public:
    static constexpr size_t MAX_SIZE = 256;

    using Iterator = IdSet::const_iterator;


    virtual ~DataListBase() noexcept;

    //
    // Gets an iterator for all IDs in use by this list
    //
    Iterator begin() const noexcept;

    void clear() noexcept;

    Iterator end() const noexcept;

    //
    // total count of items in the list
    //
    size_t size() const noexcept;

    //
    // size of the item vector in use
    //
    size_t capacity() const noexcept;

    //
    // gets the next available id to insert/duplicate an item into
    //
    uint8_t nextAvailableId() const noexcept;
    
    DataItem* insert();

    DataItem* insert(uint8_t id);

    DataItem* duplicate(uint8_t id);

    DataItem* get(uint8_t id) const;

    std::shared_ptr<DataItem> getShared(uint8_t id) const;

    void remove(uint8_t id);

protected:
    
    DataListBase() noexcept;

    virtual std::shared_ptr<DataItem> createItem() = 0;

    virtual std::shared_ptr<DataItem> copyItem(DataItem const& item) = 0;

private:

    void findNextId();

    DataVector mData;
    IdSet mIdsInUse;
    uint8_t mNextId;
};


//
// DataList class. Container for storing DataItems such that they are
// accessible via their id.
//
template <class T>
class DataList final : public DataListBase {

    static_assert(std::is_base_of<DataItem, T>::value, "T must inherit from DataItem");

public:

    DataList();
    ~DataList();

    //
    // Gets a pointer to the item with the given index if it exists. If the
    // item does not exist, nullptr is returned. The pointer may be invalidated
    // after calling insert()
    //
    T* operator[](uint8_t id) const;

    T* insert();

    T* insert(uint8_t id);

    T* duplicate(uint8_t id);

    T* get(uint8_t id) const;

    std::shared_ptr<T> getShared(uint8_t id) const;

protected:

    virtual std::shared_ptr<DataItem> createItem() override;

    virtual std::shared_ptr<DataItem> copyItem(DataItem const& item) override;

};

// we will only use these template instantiations
using InstrumentList = DataList<Instrument>;
using WaveformList = DataList<Waveform>;

}
