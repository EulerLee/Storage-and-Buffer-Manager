#include "storage.h"
#include "buffer.h"
using namespace std;

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);

    auto dsmgr = Storage::DSMgr::Get();
    auto bmgr = Buffer::BMgr::Get();

    //dsmgr->Printd();
    int type;
    int page_id;
    for(int i = 0; i < 500000; ++i) {
        //cout << "i = " << i << "\t";
        string tmp;
        cin >> tmp;
        type = stoi(tmp.substr(0, 1));
        page_id = stoi(tmp.substr(2, tmp.size()-2));
        --page_id;
        //cout << type << " " << page_id << endl;
        //if(page_id >= 50000) continue;
        bmgr->FixPage(page_id, type);
    }
}
