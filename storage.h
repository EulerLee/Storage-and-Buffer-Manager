#ifndef STORAGE
#define STORAGE

#include "environment.h"
#include "buffer.h"

namespace Storage {
    using namespace std;
    class DSMgr {
        public:
            static DSMgr* Get() {
                static DSMgr e;
                return &e;
            }

            int OpenFile();                             // Open file
            int CloseFile();                            // Close current file
            int ReadPage(int page_id, bFrame &read_frame);              // 返回读了多少字节
            int WritePage(int frame_id, bFrame frm);    // 返回写了多少字节
            int Seek(int offset, int pos);              // 移动文件指针 pos + offset
            FILE * GetFile();
            void IncNumPages();
            int GetNumPages();
            void SetUse(int page_id, int use_bit);      // 没有被使用的page(no records)，可以被reuse
            int GetUse(int page_id);

            int NewPage();                              // return page_id
            void PrintIO() {
                cout << "IO:\t" << IO << endl;
            }
            void Printd() {
                cout << numPages << endl;
                for(int i = 0; i < numPages; ++i) {
                    cout << pageoffset[i] << endl;
                }
            }
        private:
            DSMgr();
            ~DSMgr();
            FILE *currFile;
            int numPages;
            int pages[MAXPAGES];                        // use_bit
            long long pageoffset[MAXPAGES];             // page 在文件中的偏移量
            int IO;                                     // I/O次数
    };
};

#endif
