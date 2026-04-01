#pragma once

#include "Song.h"
#include "SongStatus.h"

#include <string>
#include <vector>

std::string to_string(const std::vector<Song>& songs);
std::string to_string(const SongStatus& songStatus);
std::string to_string(const std::vector<Song>& songs, const SongStatus& songStatus);
std::string to_string_with_path(const std::vector<Song>& songs);
