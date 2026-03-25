#include <gtest/gtest.h>

#include <filesystem>

#include "../src/MusicRepository.h"

struct TempMusicRepository {
	MusicRepository repository;
	std::filesystem::path temp_filename = std::filesystem::temp_directory_path() / "tempdb.db";

	TempMusicRepository() : repository{ temp_filename } {
		
	}
};

TEST(Repository, FetchAllSongs) {
	
}