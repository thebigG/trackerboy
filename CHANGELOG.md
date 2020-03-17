# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.0.3] - 2020-03-09
### Added
 - More test cases for File
 - tests for `Table<T>`
 - misc/genstep.py, similar to gensinc.py, generates bandlimited step sets for Osc
 - implementation for WaveOsc
 - NoiseGen, buffered version of NoiseChannel
 - Envelope, moved code from EnvChannel here (will replace EnvChannel)
 - Sweep class, replaces SweepPulseChannel
 - HardwareFile struct (replaces ChannelFile)
### Changed
 - Fix exception being thrown when attempting to insert into table of size one less than maximum
 - Rewrote Osc class. Subclasses must edit the delta buffer manually instead of
   using `deltaSet`.
 - Osc no longer uses sinc sets. It uses bandlimited steps instead, similiar to
   blargg's http://www.slack.net/~ant/bl-synth/11.implementation.html. `Osc::generate`
   uses floats for samples instead of int16_t. Samples range from -1.0-1.0, exceeding both
   limits due to overshoot (so volume will need to be adjusted or clipping will occur).
 - Mixer no longer emulates terminal volume/panning (tracker doesn't use it)
 - Mixer works with sample buffers as opposed to individual samples from each channel
 - Length counters are no longer emulated by the Synth
 - Mixer and Sequencer were moved into Synth, as only the synth used these internally.
### Removed
  - `Osc::deltaSet` as only WaveOsc needed this method
  - `Osc::setMute` and `Osc::muted`, replaced by `Osc::disable` and `Osc::disabled`
  - misc/gensinc.py no longer needed, replaced by genstep.py
  - Channel, EnvChannel, PulseChannel, SweepPulseChannel, WaveChannel, NoiseChannel,
    ChannelFile, Mixer and Sequencer classes

## [0.0.2] - 2020-02-05
### Added
 - `File::loadTable<T>` method
 - uint16_t overload for correctEndian
 - Destructor and copy constructor to `Table<T>`
 - `Table<T>::insert` methods
 - `Table<T>::clear`
 - Version struct + operator overloads
### Changed
 - Rewrote implementation for `Table<T>`, no longer uses `std::unordered_map`
   Uses a vector for the item data, and a 256 uint8_t array that maps an id
   to an index in the vector. Allowing for faster lookups but a slight
   performance loss when removing items.
 - Refactored pattern_demo.cpp and File.cpp to use new Table implementation
 - table size is now a 2 byte field when saving/loading
 - add setSpeed() overload to calculate speed from tempo/rpb settings. Song
   no longer calculates speed when tempo or rpb is set.
### Removed
 - `Table<T>::add`, `Table<T>::set`, as these methods are no longer needed as the
   insert methods should be used instead.

## [0.0.1] - 2020-02-01
### Added
 - This CHANGELOG.md file to serve as a changelog for any new features, removals
   deprecations and so on for the project as a whole.
### Changed
 - README.md, added a roadmap section to list planned features and the order
   in which they are worked on.
 - ORGANIZATION.md, added a guideline to use unix style newlines in all files
 - CMakeLists.txt, updated project version to v0.0.1