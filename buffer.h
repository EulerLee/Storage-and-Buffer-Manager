#ifndef BUFFER
#define BUFFER

#include "environment.h"
#include "storage.h"

namespace Buffer {
    using namespace std;

    static bFrame buf[DEFBUFSIZE]; // 1024*4096*8 bytes, buffer pool

    // buffer control block, information of frame
    struct BCB
    {
        BCB() = default;
        int page_id;
        int frame_id;
        int count;      // pin-count
        int dirty;
        BCB * next;
    };

    /*
     LRU
    */
    struct LRUNode
    {
        int frame_id;
        LRUNode *next;
        LRUNode *pre;
        LRUNode(int frame_id = 0) : frame_id(frame_id), next(nullptr), pre(nullptr) {}
    };

    struct LRUList
    {
        LRUNode *head;
        int count;
        LRUList() : head(nullptr), count(0) {}
        /*~LRUList() {
            LRUNode *p = head->next;
            delete head;
            while(p) {
                head = p;
                p = p->next;
                delete head;
            }
        }*/

        //void insert(LRUNode *node);
        void insert(int frame_id);
        LRUNode* find_node(int frame_id);
        int pop();
        int get_tail();
    };


    class BMgr
    {
        public:
            static BMgr* Get() {
                static BMgr e;
                return &e;
            }
            // Interface functions
            /*
             * 返回 frame_id, 如果page已经在buffer中
             * 否则，选择一个buffer中的页替换掉（LRU）
             * prot 表示读或写
            */
            int FixPage(int page_id, int prot);
            /*
             * 创建新的page, 返回对应的 page_id
            */
            int FixNewPage();
            /*
             * 减少page_id对应的frame上 pin-count 的值
            */
            int UnfixPage(int page_id);
            /*
             * buffer 中可用的页数
            */
            int NumFreeFrames();
            // Internal Functions
            int SelectVictim();                     // 选一个内存页置换，如果是 dirty 的，需要先写回
            int Hash(int page_id);                  // 返回frame_id
            void RemoveBCB(BCB* ptr, int page_id);  // 删除 page_id 对应的 BCB, 内存页置换的时候调用
            void RemoveLRUEle(int frame_id);        // 从 LRUList 中删去 LRU 元素
            void SetDirty(int frame_id);
            void UnsetDirty(int frame_id);
            void WriteDirtys();                     // called when the system is shut down
            void PrintFrame(int frame_id);          // 打印 frame_id 对应的内存页的内容

            int getpageid(int frame_id) {
                return ftop[frame_id];
            }
        private:
            BMgr();
            ~BMgr();
            // Hash Table
            int ftop[DEFBUFSIZE];
            BCB* ptof[DEFBUFSIZE];
            // LRU list
            LRUList list_LRU;

            int numFree;

            int cachemiss;
    };
};

#endif
