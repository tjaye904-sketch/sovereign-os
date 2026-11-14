
#include "audit_log.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>
using namespace sovereign::audit;
int main(){
  auto dir=std::filesystem::temp_directory_path()/\"sov_audit_test\";
  std::error_code ec; std::filesystem::remove_all(dir,ec);
  AuditEntryId id{};
  { AuditLog a; REQUIRE(a.Open(dir)==AuditError::Ok);
    AuditEntry e; e.category=\"test\"; e.action=\"init\";
    auto s=a.Append(e); REQUIRE(s.Ok()); id=s.value;
    auto list=a.List(10); REQUIRE(list.Ok()); REQUIRE(!list.value.empty());
    a.Close(); }
  { AuditLog a; REQUIRE(a.Open(dir)==AuditError::Ok);
    auto list=a.ListByCategory(\"test\",10); REQUIRE(list.Ok());
    REQUIRE(!list.value.empty());
    auto chain=a.ChainTip(); REQUIRE(chain.value==\"genesis\");
    a.Close(); }
  std::filesystem::remove_all(dir,ec);
  std::cout<<\"audit_log_smoke_test: OK\\n\"; return 0;
}
