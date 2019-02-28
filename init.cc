#include "storage.h"
#include "buffer.h"
using namespace std;

int main()
{
    auto dsmgr = Storage::DSMgr::Get();
    auto bmgr = Buffer::BMgr::Get();
    while(dsmgr->GetNumPages() < 50000) {
        (*bmgr).FixNewPage();
    }
}
