
#include "archive_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>
using namespace sovereign::archive;
int main(){
  auto dir=std::filesystem::temp_directory_path()/"sov_arch_test";
  std::error_code ec; std::filesystem::remove_all(dir,ec);
  ArtifactId id{};
  { ArchiveService a; REQUIRE(a.Open(dir)==ArchiveError::Ok);
    Artifact art; art.content_hash="abc123"; art.label="build-evidence";
    auto s=a.Store(art); REQUIRE(s.Ok()); id=s.value;
    REQUIRE(a.Get(id).value.label=="build-evidence"); a.Close(); }
  { ArchiveService a; REQUIRE(a.Open(dir)==ArchiveError::Ok);
    REQUIRE(a.Get(id).value.content_hash=="abc123"); a.Close(); }
  std::filesystem::remove_all(dir,ec);
  std::cout<<"archive_service_smoke_test: OK\n"; return 0;
}
