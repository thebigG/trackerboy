
#include "trackerboy/data/Module.hpp"
#include "trackerboy/fileformat.hpp"

#include "./checkedstream.hpp"
#include "internal/endian.hpp"
#include "internal/tlv.hpp"

#include <algorithm>
#include <cstddef>


namespace trackerboy {


Module::Module() noexcept :
    mSongs(),
    mInstrumentList(),
    mWaveformList(),
    mVersion(VERSION),
    mRevision(FILE_REVISION),
    mTitle(""),
    mArtist(""),
    mCopyright("") 
{
}

Module::~Module() noexcept {
}

void Module::clear() noexcept {
    mVersion = VERSION;
    mRevision = FILE_REVISION;
    mTitle = "";
    mArtist = "";
    mCopyright = "";
    mSongs.clear();
    mInstrumentList.clear();
    mWaveformList.clear();
}

std::string Module::artist() const noexcept {
    return mArtist;
}

std::string Module::title() const noexcept {
    return mTitle;
}

std::string Module::copyright() const noexcept {
    return mCopyright;
}

Version Module::version() const noexcept {
    return mVersion;
}

uint8_t Module::revision() const noexcept {
    return mRevision;
}

std::vector<Song>& Module::songs() noexcept {
    return mSongs;
}

InstrumentList& Module::instrumentList() noexcept {
    return mInstrumentList;
}

WaveformList& Module::waveformList() noexcept {
    return mWaveformList;
}

void Module::setArtist(std::string artist) noexcept {
    mArtist = artist;
}

void Module::setCopyright(std::string copyright) noexcept {
    mCopyright = copyright;
}

void Module::setTitle(std::string title) noexcept {
    mTitle = title;
}

// ---- Serialization ----

enum class FormatCmd : uint8_t {
    songAppend = 'S',           // 'S'
    instrumentInsert = 'I',     // 'I' <id>: inserts an instrument with the given id
    waveInsert = 'W',           // 'W' <id> -> W/1/<id>
    name = 'N',                 // 'N' <name>: sets the name for the last added DataItem (S I or W must preceed this command!)
    comments = 'C',             // 'C' <comments>: sets comments for the module
    // song commands (applies to song created from songAppend)
    songSpeed = 'E',            // 'E' <speed>: sets the song's speed
    songRpb = 'B',              // 'B' <rpb>: sets the song's rows per beat
    songRpm = 'M',              // 'M' <rpm>: sets the song's rows per measure
    songAppendPattern = 'P',    // 'P' <order>: appends a pattern to the song's order
    songSelectTrack = 'T',      // 'T' <ch> <trackId>: selects a track to edit
    songSetRow = 'R',           // 'R' <rowno> <rowdata>: sets rowdata for the selected track
    // waveform commands
    waveSet = 'w',              // 'w' <wavedata>: sets waveform data
    // instrument commands
    instrumentSet = 'i'         // 'i' <instrumentData>: sets instrument data

};

// command argument structs

#pragma pack(push, 1)

struct SelectTrackArgs {
    uint8_t channel;
    uint8_t trackId;
};

struct SetRowArgs {
    uint8_t rowno;
    TrackRow rowdata;
};

#pragma pack(pop)

constexpr uint8_t operator+(FormatCmd lhs) {
    return static_cast<uint8_t>(lhs);
}


FormatError Module::deserialize(std::istream &stream) noexcept {

    // read in the header
    Header header;
    checkedRead(stream, &header, sizeof(header));

    // check the signature
    if (!std::equal(header.signature, header.signature + Header::SIGNATURE_SIZE, FILE_SIGNATURE)) {
        return FormatError::invalidSignature;
    }

    // check revision, do not attempt to parse future versions
    if (header.revision > FILE_REVISION) {
        return FormatError::invalidRevision;
    }

    // check file type for module
    if (static_cast<FileType>(header.type) != FileType::mod) {
        return FormatError::invalidType;
    }

    mRevision = header.revision;

    // ensure strings are null terminated
    header.title[Header::TITLE_LENGTH - 1] = '\0';
    header.artist[Header::ARTIST_LENGTH - 1] = '\0';
    header.copyright[Header::COPYRIGHT_LENGTH - 1] = '\0';

    mTitle = std::string(header.title);
    mArtist = std::string(header.artist);
    mCopyright = std::string(header.copyright);

    FormatError error;

    // song list
    uint8_t songCount;
    checkedRead(stream, &songCount, 1);
    size_t adjSongCount = static_cast<size_t>(songCount) + 1;
    mSongs.clear();
    mSongs.resize(adjSongCount);
    for (size_t i = 0; i != adjSongCount; ++i) {
        auto &song = mSongs[i];
        error = song.deserialize(stream);
        if (error != FormatError::none) {
            return error;
        }
    }

    // instrument table
    //error = mInstrumentTable.deserialize(stream);
    //if (error != FormatError::none) {
    //    return error;
    //}

    //// wave table
    //error = mWaveTable.deserialize(stream);


    return error;
}

FormatError Module::serialize(std::ostream &stream) noexcept {
    

    // setup the header

    Header header{ 0 };

    // signature
    std::copy(FILE_SIGNATURE, FILE_SIGNATURE + Header::SIGNATURE_SIZE, header.signature);

    // version information (saving always overrides what was loaded)
    header.versionMajor = correctEndian(mVersion.major);
    header.versionMinor = correctEndian(mVersion.minor);
    header.versionPatch = correctEndian(mVersion.patch);

    // file information

    // revision remains the same as the one that was loaded.
    // for new files, it is set to the current revision.
    header.revision = mRevision;
    header.type = static_cast<uint8_t>(FileType::mod);

    #define copyStringToFixed(dest, string, count) do { \
            size_t len = std::min(count - 1, string.length()); \
            string.copy(dest, len); \
            dest[len] = '\0'; \
        } while (false)

    copyStringToFixed(header.title, mTitle, Header::TITLE_LENGTH);
    copyStringToFixed(header.artist, mArtist, Header::ARTIST_LENGTH);
    copyStringToFixed(header.copyright, mCopyright, Header::COPYRIGHT_LENGTH);

    #undef copyStringToFixed

    // reserved was zero'd on initialization of header

    // write the header
    checkedWrite(stream, &header, sizeof(header));
    
    try {

        // write comments if set

        // write songs
        for (auto &song : mSongs) {
            tlvparser::write(stream, +FormatCmd::songAppend);

            auto name = song.name();
            tlvparser::write(stream, +FormatCmd::name, (uint32_t)name.length(), name.c_str());

            tlvparser::write(stream, +FormatCmd::songSpeed, song.speed());

            tlvparser::write(stream, +FormatCmd::songRpb, song.rowsPerBeat());

            tlvparser::write(stream, +FormatCmd::songRpm, song.rowsPerMeasure());

            // order
            auto &order = song.orders();
            for (auto &pattern : order) {
                tlvparser::write(stream, +FormatCmd::songAppendPattern, pattern);
            }

            // pattern data
            auto &pm = song.patterns();
            // iterate all channels
            for (uint8_t ch = 0; ch <= static_cast<uint8_t>(ChType::ch4); ++ch) {
                // track iterators for the current channel
                auto begin = pm.tracksBegin(static_cast<ChType>(ch));
                auto end = pm.tracksEnd(static_cast<ChType>(ch));

                // iterate all tracks for this channel
                for (auto pair = begin; pair != end; ++pair) {
                    // make sure track is non-empty
                    if (pair->second.rowCount() > 0) {
                        SelectTrackArgs selectTrackArgs = { ch, pair->first };
                        tlvparser::write(stream, +FormatCmd::songSelectTrack, selectTrackArgs);

                        // iterate all rows in this track
                        uint8_t rowno = 0;
                        SetRowArgs setRowArgs;
                        for (auto &row : pair->second) {
                            if (row.flags) {
                                setRowArgs = { rowno, row };
                                tlvparser::write(stream, +FormatCmd::songSetRow, setRowArgs);
                            }

                            ++rowno;
                        }
                    }
                }
            }

        }

        // write instruments
        // TODO

        // write waveforms
        // TODO


    } catch (tlvparser::IOError const& err) {
        (void)err;
        return FormatError::writeError;
    }

    return FormatError::none;
}

}
