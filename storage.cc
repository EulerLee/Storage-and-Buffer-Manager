#include "storage.h"
using namespace Storage;

DSMgr::DSMgr() : IO(0), numPages(0)
{
    assert(OpenFile());
}

DSMgr::~DSMgr()
{
    PrintIO();
    assert(CloseFile());
}

int DSMgr::OpenFile()
{
    currFile = fopen("./datafile/data.dbf", "rb+");
    if(currFile == NULL) {
        /*cerr << "DSMgr: Open file failed" << endl;
        return 0;*/
        currFile = fopen("./datafile/data.dbf", "wb+");
        if(fwrite(&numPages, sizeof(int), 500*4096/4, currFile) != 500*4096/4) {
            cerr << "DSMgr: init dbf failed" << endl;
            return 0;
        }
        fclose(currFile);
        currFile = fopen("./datafile/data.dbf", "rb+");
        cout << numPages << endl;
        return 1;
    }else {
        //dbf format: numPages, pageoffset, pages(use_bit) (first 500 pages)
        if(fread(&numPages, sizeof(int), 1, currFile) != 1) {
            cerr << "DSMgr: init numPages failed" << endl;
            return 0;
        }
        if(numPages == 0) {
            return 1;
        }
        if(fread(pageoffset, sizeof(long long)*numPages, 1, currFile) != 1) {
            cerr << "DSMgr: init pageoffset failed" << endl;
            return 0;
        }
        if(fread(pages, sizeof(int)*numPages, 1, currFile) != 1) {
            cerr << "DSMgr: init pages failed" << endl;
            return 0;
        }
        return 1;
    }
}

// This function should only be called as the database is changed or a the program closes.
int DSMgr::CloseFile()
{
    fseek(currFile, 0, SEEK_SET);
    if(fwrite(&numPages, sizeof(int), 1, currFile) != 1) {
        cerr << "DSMgr: Write numPages failed" << endl;
        return 0;
    }
    if(fwrite(pageoffset, sizeof(long long)*numPages, 1, currFile) != 1) {
        cerr << "DSMgr: Write pageoffset failed" << endl;
        return 0;
    }
    if(fwrite(pages, sizeof(int)*numPages, 1, currFile) != 1) {
        cerr << "DSMgr: Write pages failed" << endl;
        return 0;
    }
    if(fclose(currFile) == 0) {
        return 1;
    }else {
        cerr << "DSMgr: Close file failed" << endl;
        return 0;
    }
}

// We don't actually need to complete it
int DSMgr::ReadPage(int page_id, bFrame &read_frame)
{
    long long offset = pageoffset[page_id];
    //cout << offset << endl;
    fseek(currFile, offset, SEEK_SET);
    if(fread(&read_frame, sizeof read_frame, 1, currFile) != 1) {
        cerr << "DSMgr: Read page failed" << endl;
        return 0;
    }

    ++IO;
    return sizeof(read_frame);
}

/*bFrame* DSMgr::ReadPage(int page_id)
{
    int offset = pages[page_id];
    fseek(currFile, offset, SEEK_SET);
    bFrame *read_res = new bFrame;      // memory leak
    if(fread(read_res, sizeof read_res, 1, currFile) != 1) {
        cerr << "DSMgr: Read page failed" << endl;
        return nullptr;
    }
    return read_res;
}*/

int DSMgr::WritePage(int frame_id, bFrame frm)
{
    auto bmgr = Buffer::BMgr::Get();
    int page_id = bmgr->getpageid(frame_id);
    long long offset = pageoffset[page_id];
    fseek(currFile, offset, SEEK_SET);
    if(fwrite(&frm, sizeof frm, 1, currFile) != 1) {
        cerr << "DSMgr: Write page failed" << endl;
        return 0;
    }
    ++IO;
    return sizeof(frm);
}

int DSMgr::Seek(int offset, int pos)
{
    fseek(currFile, pos+offset, SEEK_SET);
    return 0;
}

inline FILE* DSMgr::GetFile()
{
    return currFile;
}

inline void DSMgr::IncNumPages()
{
    ++numPages;
}

int DSMgr::GetNumPages()
{
    return numPages;
}

inline void DSMgr::SetUse(int index, int use_bit)
{
    pages[index] = use_bit;
}

inline int DSMgr::GetUse(int index)
{
    return pages[index];
}

int DSMgr::NewPage()
{
    /*
    FindUse
    */

    ++numPages;
    int page_id = numPages-1;
    pages[page_id] = 0;
    fseek(currFile, 0, SEEK_END);
    pageoffset[page_id] = ftell(currFile);
    cout << pageoffset[page_id] << endl;
    bFrame frm;
    if(fwrite(&frm, sizeof frm, 1, currFile) != 1) {
        cerr << "DSMgr: Create page failed" << endl;
        assert(0);
    }
    return page_id;
}
