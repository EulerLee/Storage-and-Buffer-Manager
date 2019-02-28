#include "buffer.h"
using namespace Buffer;

/********
 LRUList
********/
/*void LRUList::insert(LRUNode *node)
{
    LRUNode* pn = find_node(node->frame_id);
    if(pn == nullptr) {
        node->next = head;
        node->pre = nullptr;
        head->pre = node;
        head = node;
        ++count;
    }else {
        pn->pre->next = pn->next;
        pn->next->pre = pn->pre;
        pn->next = head;
        pn->pre = nullptr;
        head->pre = pn;
        head = pn;
    }
}*/

void LRUList::insert(int frame_id)
{
    LRUNode* pn = find_node(frame_id);
    if(pn == nullptr) {
        if(head == nullptr) {
            head = new LRUNode;
            head->frame_id = frame_id;
        }else {
            LRUNode *node = new LRUNode;
            node->frame_id = frame_id;
            node->next = head;
            head->pre = node;
            head = node;
            head->pre = nullptr;
        }
        ++count;
    }else {
        if(head->frame_id == frame_id) {
            return;
        }
        pn->pre->next = pn->next;
        if(pn->next != nullptr) {
            pn->next->pre = pn->pre;
        }
        pn->next = head;
        pn->pre = nullptr;
        head->pre = pn;
        head = pn;
    }
}

LRUNode* LRUList::find_node(int frame_id)
{
    LRUNode *pn = head;
    while(pn) {
        if(pn->frame_id == frame_id) {
            return pn;
        }
        pn = pn->next;
    }
    return nullptr;
}

int LRUList::pop()
{
    LRUNode* pn = head;
    while(pn->next) {
        pn = pn->next;
    }
    int vic = pn->frame_id;
    pn->pre->next = nullptr;
    delete pn;
    --count;
    return vic;
}

int LRUList::get_tail()
{
    if(head == nullptr) {
        return -1;
    }
    LRUNode* pn = head;
    while(pn->next) {
        pn = pn->next;
    }
    int vic = pn->frame_id;
    return vic;
}
/*********
   BMgr
*********/
BMgr::BMgr() : numFree(DEFBUFSIZE), list_LRU(LRUList()), cachemiss(0)
{
    // initalize the buffer pool
    memset(buf, 0, sizeof buf);
    // initalize the Hash table
    memset(ftop, 0, sizeof ftop);
    memset(ptof, 0, sizeof ptof);
}

BMgr::~BMgr()
{
    WriteDirtys();
    cout << "cache miss rate:\t" << (cachemiss+0.0)/500000 << endl;
}


int BMgr::FixPage(int page_id, int prot)
{
    auto dsmgr = Storage::DSMgr::Get();
    if(page_id == (*dsmgr).GetNumPages()) {
        FixNewPage();
    }
    BCB *pb = ptof[Hash(page_id)];
    while(pb) {
        if(pb->page_id == page_id) {
            break;
        }
        pb = pb->next;
    }
    if(pb == nullptr) {         // page not in buffer
        int victim = SelectVictim();
        ++cachemiss;
        assert((*dsmgr).ReadPage(page_id, buf[victim]));
        ftop[victim] = page_id;
        if(ptof[Hash(page_id)] == nullptr) {
            ptof[Hash(page_id)] = new BCB;
            pb = ptof[Hash(page_id)];
            pb->page_id = page_id;
            pb->frame_id = victim;
            pb->count = 1;
            pb->dirty = prot;
            pb->next = nullptr;
        }else {
            pb = ptof[Hash(page_id)];
            while(pb->next != nullptr) {
                pb = pb->next;
            }
            pb->next = new BCB;
            pb = pb->next;
            pb->page_id = page_id;
            pb->frame_id = victim;
            pb->count = 1;
            pb->dirty = prot;
            pb->next = nullptr;
        }
        //cout << "victim:\t" << victim << endl;
    }
    //cout << "ok" << endl;
    list_LRU.insert(pb->frame_id);
    return pb->frame_id;
}

int BMgr::FixNewPage()
{
    auto dsmgr = Storage::DSMgr::Get();
    int page_id = (*dsmgr).NewPage();
    return page_id;
}

int BMgr::UnfixPage(int page_id)
{
    BCB* pb = ptof[Hash(page_id)];
    while(pb) {
        if(pb->page_id == page_id) {
            break;
        }
        pb = pb->next;
    }
    if(pb != nullptr) {
        if(pb->count > 0) {
            --(pb->count);
            return pb->frame_id;
        }else {
            cerr << "BMgr: UnfixPage failed: pin-count < 0" << endl;
            assert(0);
        }
    }else {
        cerr << "BMgr: UnfixPage failed: page_id not found" << endl;
        assert(0);
    }
}

int BMgr::NumFreeFrames()
{
    /*int res = DEFBUFSIZE;
    for(int i = 0; i < DEFBUFSIZE; ++i) {
        BCB* pb = ptof[i];
        while(pb) {
            --res;
            pb = pb->next;
        }
        if(res == 0) break;
    }
    return res;*/
    return numFree;
}

int BMgr::SelectVictim()
{
    int nff = NumFreeFrames();
    if(nff > 0) {               // Buffer 有空余
        --numFree;
        int frame_id = DEFBUFSIZE - nff;
        return frame_id;
    }else {                     // Buffer 已满
        auto dsmgr = Storage::DSMgr::Get();
        int vic = list_LRU.get_tail();
        //cout << "vic:\t" << vic << endl;
        int page_id = ftop[vic];
        BCB* pb = ptof[Hash(page_id)];
        /*if(pb == nullptr) {
            cout << "odd:\t" << page_id << endl;
            for(int i = 0; i < DEFBUFSIZE; ++i) {
                if(ptof[i] == nullptr) {
                    cout << i << ":\tnull" << endl;
                }
            }
        }*/
        while(pb) {
            if(pb->page_id == page_id) {
                break;
            }
            pb = pb->next;
        }
        if(pb == nullptr) {
            cerr << "BMgr: Select Victim failed: BCB not found" << endl;
            assert(0);
        }
        if(pb->dirty) {
            assert((*dsmgr).WritePage(pb->frame_id, buf[pb->frame_id]));
        }
        RemoveBCB(ptof[Hash(page_id)], page_id);
        RemoveLRUEle(vic);
        return vic;
    }
}

// Hash function ptof
inline int BMgr::Hash(int page_id)
{
    return page_id % DEFBUFSIZE;
}

void BMgr::RemoveBCB(BCB* ptr, int page_id)
{
    if(ptr == nullptr) {
        cerr << "BMgr: Remove BCB failed: page_id not found" << endl;
        assert(0);
    }
    // 在桶里找page_id
    BCB* pre = nullptr;
    while(ptr->page_id != page_id && ptr != nullptr) {
        pre = ptr;
        ptr = ptr->next;
    }
    if(ptr == nullptr) {
        cerr << "BMgr: Remove BCB failed: page_id not found" << endl;
        assert(0);
    }else {
        if(pre == nullptr) {    // page_id is the first
            ptof[Hash(page_id)] = ptr->next;
            delete ptr;
        }else {
            pre->next = pre->next->next;
            delete ptr;
        }
    }
}

void BMgr::RemoveLRUEle(int frame_id)
{
    LRUNode* pb = list_LRU.head;
    while(pb) {
        if(pb->frame_id == frame_id) {
            break;
        }
        pb = pb->next;
    }
    if(pb == nullptr) {
        cerr << "BMgr: Erase LRU element failed: frame_id not found" << endl;
        return;
    }else {
        if(pb->pre == nullptr) {    // pb 是头节点
            list_LRU.head = pb->next;
            list_LRU.head->pre = nullptr;
        }else {
            pb->pre->next = pb->next;
            if(pb->next != nullptr) {
                pb->next->pre = pb->pre;
            }
        }
        delete pb;
        list_LRU.count--;
    }
}

void BMgr::SetDirty(int frame_id)
{
    int page_id = ftop[frame_id];
    BCB* pb = ptof[Hash(page_id)];
    while(pb) {
        if(pb->frame_id == frame_id) {
            break;
        }
        pb = pb->next;
    }
    if(pb == nullptr) {
        cerr << "BMgr: Set Dirty failed: frame_id not found" << endl;
        assert(0);
    }
    pb->dirty = 1;
}

void BMgr::UnsetDirty(int frame_id)
{
    int page_id = ftop[frame_id];
    BCB* pb = ptof[Hash(page_id)];
    while(pb) {
        if(pb->frame_id == frame_id) {
            break;
        }
        pb = pb->next;
    }
    if(pb == nullptr) {
        cerr << "BMgr: Unset Dirty failed: frame_id not found" << endl;
        return;
    }
    pb->dirty = 0;
}

void BMgr::WriteDirtys()
{
    auto dsmgr = Storage::DSMgr::Get();
    // Traverse the BCB list
    for(int i = 0; i < DEFBUFSIZE; ++i) {
        BCB* pb = ptof[i];
        while(pb) {
            if(pb->dirty == 1) {
                assert((*dsmgr).WritePage(pb->frame_id, buf[pb->frame_id]));
            }
            pb = pb->next;
        }
    }
}

void BMgr::PrintFrame(int frame_id)
{
    bFrame frm = buf[frame_id];
    for(int i = 0; i < FRAMESIZE; ++i) {
        cout << frm.field[i];
    }
    cout << endl;
}
